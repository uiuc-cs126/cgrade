/*
 * Author(s): ljeabmreosn.
 */
#ifndef LIB_CATCH_REPORTER_GRADESCOPE_H_
#define LIB_CATCH_REPORTER_GRADESCOPE_H_

#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

namespace Catch {

class GradescopeReporter : public CumulativeReporterBase<GradescopeReporter> {
 public:
  GradescopeReporter(const ReporterConfig& config);
  ~GradescopeReporter() override;

  static std::string getDescription();

  void noMatchingTestCases(const std::string& spec) override;
  void testRunStarting(const TestRunInfo& runInfo) override;
  void testGroupStarting(const GroupInfo& groupInfo) override;
  void testCaseStarting(const TestCaseInfo& testCaseInfo) override;
  bool assertionEnded(const AssertionStats& assertionStats) override;
  void testCaseEnded(const TestCaseStats& testCaseStats) override;
  void testGroupEnded(const TestGroupStats& testGroupStats) override;
  void testRunEndedCumulative() override;

  void writeGroup(const TestGroupNode& groupNode, double suiteTime);
  void writeTestCase(const TestCaseNode& testCaseNode);
  void writeAssertions(const SectionNode& sectionNode);
  void writeAssertion(const AssertionStats& stats);
  void writeSection(
    const std::string& className,
    const std::string& rootName,
    const SectionNode& sectionNode
  );

 private:
  XmlWriter xml;
  Timer suiteTimer;
  std::string stdOutForSuite;
  std::string stdErrForSuite;
  unsigned int unexpectedExceptions = 0;
  bool m_okToFail = false;
};

} // end namespace Catch

#include "catch_reporter_gradescope.hpp"

#endif // LIB_CATCH_REPORTER_GRADESCOPE_H_
