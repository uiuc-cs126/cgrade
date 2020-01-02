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
    GradescopeReporter(ReporterConfig const& _config);

    ~GradescopeReporter() override;

    static std::string getDescription();

    void noMatchingTestCases(std::string const& /*spec*/) override;
    void testRunStarting(TestRunInfo const& runInfo) override;
    void testGroupStarting(GroupInfo const& groupInfo) override;
    void testCaseStarting(TestCaseInfo const& testCaseInfo) override;
    bool assertionEnded(AssertionStats const& assertionStats) override;
    void testCaseEnded(TestCaseStats const& testCaseStats) override;
    void testGroupEnded(TestGroupStats const& testGroupStats) override;
    void testRunEndedCumulative() override;

    void writeGroup(TestGroupNode const& groupNode, double suiteTime);
    void writeTestCase(TestCaseNode const& testCaseNode);
    void writeAssertions(SectionNode const& sectionNode);
    void writeAssertion(AssertionStats const& stats);
    void writeSection(std::string const& className,
                    std::string const& rootName,
                    SectionNode const& sectionNode);

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
