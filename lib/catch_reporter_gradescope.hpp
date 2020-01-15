#define CATCH_CONFIG_EXTERNAL_INTERFACES

#include <algorithm>
#include <cassert>
#include <ctime>
#include <sstream>

#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

#include "catch_reporter_gradescope.h"

namespace Catch {

GradescopeReporter::GradescopeReporter(const ReporterConfig& config)
  : CumulativeReporterBase(config) {
    m_reporterPrefs.shouldRedirectStdOut = true;
    m_reporterPrefs.shouldReportAllAssertions = true;
  }

GradescopeReporter::~GradescopeReporter() { }

void GradescopeReporter::noMatchingTestCases(const std::string& spec) { }

std::string GradescopeReporter::getDescription() {
  return "Reports test results in a JSON format that corresponds with Gradescope's autograder format.";
}

void GradescopeReporter::testCaseStarting(const TestCaseInfo& testCaseInfo) {
  suiteTimer.start();
  unexpectedExceptions = 0;
  m_okToFail = testCaseInfo.okToFail();
}

bool GradescopeReporter::assertionEnded(const AssertionStats& assertionStats) {
  if (assertionStats.assertionResult.getResultType() == ResultWas::ThrewException && !m_okToFail) {
    unexpectedExceptions++;
  }
  return CumulativeReporterBase::assertionEnded(assertionStats);
}

void GradescopeReporter::testCaseEnded(const TestCaseStats& testCaseStats) {
  const double suiteTime = suiteTimer.getElapsedSeconds();
  CumulativeReporterBase::testCaseEnded(testCaseStats);
  writeTestCase(*m_testCases.back());
}

void GradescopeReporter::testRunEndedCumulative() {
  using json = nlohmann::json;
  json root;

  auto& tests = root["tests"] = json::array();

  for (const gradescope::TestCase& testCase : testCases_) {
    tests.push_back({
      { "score", testCase/*  */.score },
      { "max_score", testCase.maxScore },
      { "name", testCase.name },
      { "visibility", gradescope::toString(testCase.visibility) },
      { "output", testCase.output },
    });
  }

  m_config->stream() << root.dump();
}

void GradescopeReporter::writeTestCase(const TestCaseNode& testCaseNode) {
  const TestCaseStats& stats = testCaseNode.value;

  // All test cases have exactly one section - which represents the
  // test case itself. That section may have 0-n nested sections.
  assert(testCaseNode.children.size() == 1);
  const SectionNode& rootSection = *testCaseNode.children.front();

  gradescope::TestCase testCase;
  testCase.name = stats.testInfo.name;
  testCase.score = stats.totals.assertions.allPassed() ? 1. : 0.;
  writeSection("", rootSection, &testCase);
  testCases_.push_back(testCase);
}

void GradescopeReporter::writeSection(
  const std::string& rootName,
  const SectionNode& sectionNode,
  gradescope::TestCase* testCase
) {
  const std::string name = (!rootName.empty() ? rootName + "/" : "") + trim(sectionNode.stats.sectionInfo.name);

  if (!sectionNode.assertions.empty() || !sectionNode.stdOut.empty() || !sectionNode.stdErr.empty()) {
    writeAssertions(name, sectionNode, testCase);
  }

  for (const auto& childNode : sectionNode.childSections) {
    writeSection(name, *childNode, testCase);
  }
}

void GradescopeReporter::writeAssertions(
  const std::string& name,
  const SectionNode& sectionNode,
  gradescope::TestCase* testCase
) {
  for (const AssertionStats& assertion : sectionNode.assertions) {
    writeAssertion(name, assertion, testCase);
  }
}

void GradescopeReporter::writeAssertion(
  const std::string& name,
  const AssertionStats& stats,
  gradescope::TestCase* testCase
) {
  const AssertionResult& result = stats.assertionResult;
  if (result.isOk()) return;

  const std::string elementName = [&]() -> std::string {
    switch (result.getResultType()) {
      case ResultWas::ThrewException:
      case ResultWas::FatalErrorCondition:
        return "error";

      case ResultWas::ExplicitFailure:
      case ResultWas::ExpressionFailed:
      case ResultWas::DidntThrowException:
        return "failure";

      // We should never see these here:
      case ResultWas::Info:
      case ResultWas::Warning:
      case ResultWas::Ok:
      case ResultWas::Unknown:
      case ResultWas::FailureBit:
      case ResultWas::Exception:
        return "internalError";
    }
  }();

  // Same format as junit reporter.
  ReusableStringStream rss;
  if (stats.totals.assertions.total() > 0) {
    rss << "FAILED '" << name << "':\n";
    if (result.hasExpression()) {
      rss << "  ";
      rss << result.getExpressionInMacro();
      rss << '\n';
    }
    if (result.hasExpandedExpression()) {
      rss << "with expansion:\n";
      rss << Column(result.getExpandedExpression()).indent(2) << '\n';
    }
  } else {
    rss << '\n';
  }

  if (!result.getMessage().empty()) {
    rss << result.getMessage() << '\n';
  }

  for (const MessageInfo& msg : stats.infoMessages) {
    if (msg.type == ResultWas::Info) {
      rss << msg.message << '\n';
    }
  }

  testCase->output += rss.str();
}

CATCH_REGISTER_REPORTER("gradescope", GradescopeReporter);

} // end namespace Catch