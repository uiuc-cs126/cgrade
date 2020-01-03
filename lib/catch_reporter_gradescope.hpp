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
  : CumulativeReporterBase(config)
  , xml(config.stream())
  {
    m_reporterPrefs.shouldRedirectStdOut = true;
    m_reporterPrefs.shouldReportAllAssertions = true;
  }

GradescopeReporter::~GradescopeReporter() { }

std::string GradescopeReporter::getDescription() {
  return "Reports test results in an XML format that looks like Ant's junitreport target";
}

void GradescopeReporter::noMatchingTestCases(const std::string& spec) { }

void GradescopeReporter::testRunStarting(const TestRunInfo& runInfo)  {
  CumulativeReporterBase::testRunStarting(runInfo);
  xml.startElement("testsuites");
}

void GradescopeReporter::testGroupStarting(const GroupInfo& groupInfo) {
  suiteTimer.start();
  stdOutForSuite.clear();
  stdErrForSuite.clear();
  unexpectedExceptions = 0;
  CumulativeReporterBase::testGroupStarting( groupInfo );
}

void GradescopeReporter::testCaseStarting(const TestCaseInfo& testCaseInfo) {
  m_okToFail = testCaseInfo.okToFail();
}

bool GradescopeReporter::assertionEnded( const AssertionStats& assertionStats ) {
    if( assertionStats.assertionResult.getResultType() == ResultWas::ThrewException && !m_okToFail )
        unexpectedExceptions++;
    return CumulativeReporterBase::assertionEnded( assertionStats );
}

void GradescopeReporter::testCaseEnded( const TestCaseStats& testCaseStats ) {
    stdOutForSuite += testCaseStats.stdOut;
    stdErrForSuite += testCaseStats.stdErr;
    CumulativeReporterBase::testCaseEnded( testCaseStats );
}

void GradescopeReporter::testGroupEnded( const TestGroupStats& testGroupStats ) {
    double suiteTime = suiteTimer.getElapsedSeconds();
    CumulativeReporterBase::testGroupEnded( testGroupStats );
    writeGroup( *m_testGroups.back(), suiteTime );
}

void GradescopeReporter::testRunEndedCumulative() {
    xml.endElement();
}

void GradescopeReporter::writeGroup( const TestGroupNode& groupNode, double suiteTime ) {
    XmlWriter::ScopedElement e = xml.scopedElement( "testsuite" );

    const TestGroupStats& stats = groupNode.value;
    xml.writeAttribute( "name", stats.groupInfo.name );
    xml.writeAttribute( "errors", unexpectedExceptions );
    xml.writeAttribute( "failures", stats.totals.assertions.failed-unexpectedExceptions );
    xml.writeAttribute( "tests", stats.totals.assertions.total() );
    xml.writeAttribute( "hostname", "tbd" ); // !TBD
    if( m_config->showDurations() == ShowDurations::Never )
        xml.writeAttribute( "time", "" );
    else
        xml.writeAttribute( "time", suiteTime );
    xml.writeAttribute( "timestamp", getCurrentTimestamp() );

    // Write properties if there are any
    if (m_config->hasTestFilters() || m_config->rngSeed() != 0) {
        auto properties = xml.scopedElement("properties");
        if (m_config->hasTestFilters()) {
            xml.scopedElement("property")
                .writeAttribute("name", "filters")
                .writeAttribute("value", serializeFilters(m_config->getTestsOrTags()));
        }
        if (m_config->rngSeed() != 0) {
            xml.scopedElement("property")
                .writeAttribute("name", "random-seed")
                .writeAttribute("value", m_config->rngSeed());
        }
    }

    // Write test cases
    for( const auto& child : groupNode.children )
        writeTestCase( *child );

    xml.scopedElement( "system-out" ).writeText( trim( stdOutForSuite ), XmlFormatting::Newline );
    xml.scopedElement( "system-err" ).writeText( trim( stdErrForSuite ), XmlFormatting::Newline );
}

void GradescopeReporter::writeTestCase( const TestCaseNode& testCaseNode ) {
    const TestCaseStats& stats = testCaseNode.value;

    // All test cases have exactly one section - which represents the
    // test case itself. That section may have 0-n nested sections
    assert( testCaseNode.children.size() == 1 );
    const SectionNode& rootSection = *testCaseNode.children.front();

    std::string className = stats.testInfo.className;

    if( className.empty() ) {
        className = fileNameTag(stats.testInfo.tags);
        if ( className.empty() )
            className = "global";
    }

    if ( !m_config->name().empty() )
        className = m_config->name() + "." + className;

    writeSection( className, "", rootSection );
}

void GradescopeReporter::writeSection(
  const std::string& className,
  const std::string& rootName,
  const SectionNode& sectionNode
) {
    std::string name = trim( sectionNode.stats.sectionInfo.name );
    if( !rootName.empty() )
        name = rootName + '/' + name;

    if( !sectionNode.assertions.empty() ||
        !sectionNode.stdOut.empty() ||
        !sectionNode.stdErr.empty() ) {
        XmlWriter::ScopedElement e = xml.scopedElement( "testcase" );
        if( className.empty() ) {
            xml.writeAttribute( "classname", name );
            xml.writeAttribute( "name", "root" );
        }
        else {
            xml.writeAttribute( "classname", className );
            xml.writeAttribute( "name", name );
        }
        xml.writeAttribute( "time", ::Catch::Detail::stringify( sectionNode.stats.durationInSeconds ) );

        writeAssertions( sectionNode );

        if( !sectionNode.stdOut.empty() )
            xml.scopedElement( "system-out" ).writeText( trim( sectionNode.stdOut ), XmlFormatting::Newline );
        if( !sectionNode.stdErr.empty() )
            xml.scopedElement( "system-err" ).writeText( trim( sectionNode.stdErr ), XmlFormatting::Newline );
    }
    for( const auto& childNode : sectionNode.childSections )
        if( className.empty() )
            writeSection( name, "", *childNode );
        else
            writeSection( className, name, *childNode );
}

void GradescopeReporter::writeAssertions( const SectionNode& sectionNode ) {
    for( const auto& assertion : sectionNode.assertions )
        writeAssertion( assertion );
}

void GradescopeReporter::writeAssertion( const AssertionStats& stats ) {
    const AssertionResult& result = stats.assertionResult;
    if( !result.isOk() ) {
        std::string elementName;
        switch( result.getResultType() ) {
            case ResultWas::ThrewException:
            case ResultWas::FatalErrorCondition:
                elementName = "error";
                break;
            case ResultWas::ExplicitFailure:
                elementName = "failure";
                break;
            case ResultWas::ExpressionFailed:
                elementName = "failure";
                break;
            case ResultWas::DidntThrowException:
                elementName = "failure";
                break;

            // We should never see these here:
            case ResultWas::Info:
            case ResultWas::Warning:
            case ResultWas::Ok:
            case ResultWas::Unknown:
            case ResultWas::FailureBit:
            case ResultWas::Exception:
                elementName = "internalError";
                break;
        }

        XmlWriter::ScopedElement e = xml.scopedElement( elementName );

        xml.writeAttribute( "message", result.getExpression() );
        xml.writeAttribute( "type", result.getTestMacroName() );

        ReusableStringStream rss;
        if (stats.totals.assertions.total() > 0) {
            rss << "FAILED" << ":\n";
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

        if( !result.getMessage().empty() )
            rss << result.getMessage() << '\n';
        for( const auto& msg : stats.infoMessages )
            if( msg.type == ResultWas::Info )
                rss << msg.message << '\n';

        rss << "at " << result.getSourceInfo();
        xml.writeText( rss.str(), XmlFormatting::Newline );
    }
}

CATCH_REGISTER_REPORTER("gradescope", GradescopeReporter);

} // end namespace Catch