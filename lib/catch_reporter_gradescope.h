/*
 * Author(s): ljeabmreosn.
 */
#ifndef LIB_CATCH_REPORTER_GRADESCOPE_H_
#define LIB_CATCH_REPORTER_GRADESCOPE_H_

#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

namespace Catch {
namespace gradescope {

enum class Visibility {
  HIDDEN,
  VISIBLE,
};

std::string toString(const Visibility visibility) {
  switch (visibility) {
  case Visibility::HIDDEN: return "hidden";
  case Visibility::VISIBLE: return "visible";
  }
}

struct TestCase {
  TestCase()
    : score(0.)
    , maxScore(1.)
    , visibility(Visibility::VISIBLE) { }


  // Define an order to the test cases.
  std::string number;
  double score;
  double maxScore;
  std::string name;
  std::string output;
  std::vector<std::string> tags;
  Visibility visibility;
};

} // end namespace gradescope

class GradescopeReporter : public CumulativeReporterBase<GradescopeReporter> {
 public:
  GradescopeReporter(const ReporterConfig& config);
  ~GradescopeReporter() override;

  static std::string getDescription();

  void noMatchingTestCases(const std::string& spec) override;
  void testCaseStarting(const TestCaseInfo& testCaseInfo) override;
  bool assertionEnded(const AssertionStats& assertionStats) override;
  void testCaseEnded(const TestCaseStats& testCaseStats) override;
  void testRunEndedCumulative() override;

  void writeTestCase(const TestCaseNode& testCaseNode);
  void writeAssertions(
    const std::string& name,
    const SectionNode& sectionNode,
    gradescope::TestCase* testCase
  );
  void writeAssertion(
    const std::string& name,
    const AssertionStats& stats,
    gradescope::TestCase* testCase
  );
  void writeSection(
    const std::string& rootName,
    const SectionNode& sectionNode,
    gradescope::TestCase* testCase
  );

 private:
  std::vector<gradescope::TestCase> testCases_;
  Timer suiteTimer;
  std::string stdOutForSuite;
  std::string stdErrForSuite;
  unsigned int unexpectedExceptions = 0;
  bool m_okToFail = false;
};

} // end namespace Catch

#include "catch_reporter_gradescope.hpp"

#endif // LIB_CATCH_REPORTER_GRADESCOPE_H_
