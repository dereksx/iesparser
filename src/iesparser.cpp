// Author: Artem Bishev

#include "iesparser.h"

#include "boost\regex.hpp"
#include "boost\algorithm\string.hpp"


void IESParser::Parse(std::istream& input_stream)
{
    last_added_keyword = keywords_dictionary.end();
    std::string line;

    // Parse version
    line = ReadTrimmedLine(input_stream);
    CheckEmpty(input_stream, line);
    ParseFormatVersion(line);
    if (format != Format::UNKNOWN)
    {
        line = ReadTrimmedLine(input_stream);
        CheckEmpty(input_stream, line);
    }
    else
    {
        format = Format::LM631986;
    }

    // Parse block before TILT and TILT line
    bool tilt_reached = false;
    while (!tilt_reached)
    {
        if (IsKeywordLine(line))
        {
            ParseKeywordLine(line);
        }
        else if (IsTiltLine(line))
        {
            tilt_reached = true;
            ParseTiltLine(line);
        }
        else
        {
            if (format != Format::LM631986)
            {
                throw ParsingException("Expected keyword line or TILT line", line_counter);
            }
        }
        line = ReadTrimmedLine(input_stream);
        CheckEmpty(input_stream, line);
    }
}

 
bool IESParser::KeywordAllowedByIesna02(const std::string& keyword)
{
    if (keyword == "TEST" ||
        keyword == "TESTLAB" ||
        keyword == "TESTDATE" ||
        keyword == "NEARFIELD" ||
        keyword == "MANUFAC" ||
        keyword == "LUMCAT" ||
        keyword == "LUMINAIRE" ||
        keyword == "LAMPCAT" ||
        keyword == "LAMP" ||
        keyword == "BALLAST" ||
        keyword == "BALLASTCAT" ||
        keyword == "MAINTCAT" ||
        keyword == "DISTRIBUTION" ||
        keyword == "FLASHAREA" ||
        keyword == "COLORCONSTANT" ||
        keyword == "LAMPPOSITION" ||
        keyword == "ISSUEDATE" ||
        keyword == "OTHER" ||
        keyword == "SEARCH" ||
        keyword == "MORE")
    {
        return true;
    }
    return false;
}


bool IESParser::KeywordAllowedByIesna95(const std::string& keyword)
{
    if (keyword == "TEST" ||
        keyword == "DATE" ||
        keyword == "NEARFIELD" ||
        keyword == "MANUFAC" ||
        keyword == "LUMCAT" ||
        keyword == "LUMINAIRE" ||
        keyword == "LAMPCAT" ||
        keyword == "LAMP" ||
        keyword == "BALLAST" ||
        keyword == "BALLASTCAT" ||
        keyword == "MAINTCAT" ||
        keyword == "DISTRIBUTION" ||
        keyword == "FLASHAREA" ||
        keyword == "COLORCONSTANT" ||
        keyword == "OTHER" ||
        keyword == "SEARCH" ||
        keyword == "MORE" ||
        keyword == "BLOCK" ||
        keyword == "ENDBLOCK")
    {
        return true;
    }
    return false;
}


bool IESParser::KeywordAllowedByIesna91(const std::string& keyword)
{
    if (keyword == "TEST" ||
        keyword == "DATE" ||
        keyword == "MANUFAC" ||
        keyword == "LUMCAT" ||
        keyword == "LUMINAIRE" ||
        keyword == "LAMPCAT" ||
        keyword == "LAMP" ||
        keyword == "BALLAST" ||
        keyword == "BALLASTCAT" ||
        keyword == "MAINTCAT" ||
        keyword == "DISTRIBUTION" ||
        keyword == "FLASHAREA" ||
        keyword == "COLORCONSTANT" ||
        keyword == "MORE")
    {
        return true;
    }
    return false;
}


std::string IESParser::ReadLine(std::istream& input_stream) const
{
    line_counter++;
    std::string line;
    std::getline(input_stream, line);
    return line;
}


std::string IESParser::ReadTrimmedLine(std::istream& input_stream) const
{
    std::string line = "";
    do
    {
        line = boost::trim_copy(ReadLine(input_stream));
    } while (ignore_empty_lines && input_stream && line == "");

    return line;
}


void IESParser::ParseFormatVersion(const std::string& version_string)
{
    if (version_string == "IESNA91")
    {
        format = LM631991;
    }
    else if (version_string == "IESNA:LM-63-1995")
    {
        format = LM631995;
    }
    else if (version_string == "IESNA:LM-63-2002")
    {
        format = LM632002;
    }
    else
    {
        format = UNKNOWN;
    }
}


bool IESParser::IsKeywordLine(const std::string& line) const
{
    boost::regex regex(KEYWORD_LINE_REGEX);
    return boost::regex_match(line, regex);
}


bool IESParser::IsTiltLine(const std::string& line) const
{
    boost::regex regex(TILT_LINE_REGEX);
    return boost::regex_match(line, regex);
}


void IESParser::ParseKeywordLine(const std::string& line)
{
    boost::regex regex(KEYWORD_LINE_REGEX);
    boost::smatch what;
    if (!boost::regex_search(line, what, regex))
    {
        throw ParsingException("Keyword is expected", line_counter);
    }

    std::string key = what[1];
    std::string value = what[2];

    // Check if the specified standard allows this keyword
    if (!ignore_allowed_keywords) AcceptKeyword(key);

    // Process MORE, BLOCK and ENDBLOCK keywords separately
    // For all other keywords - just add them to dictionary
    ProcessBlockKeywords(key);
    if (key == "MORE")
    {
        if (last_added_keyword == keywords_dictionary.end())
        {
            throw ParsingException(
                "Keyword MORE occured before any other keyword", line_counter);
        }
        last_added_keyword->second += ("\n" + value);
    }
    else
    {
        keywords_dictionary[key] = value;
    }
}


void IESParser::ParseTiltLine(const std::string& line)
{
    boost::regex regex(TILT_LINE_REGEX);
    boost::smatch what;
    if (!boost::regex_search(line, what, regex))
    {
        throw ParsingException("TILT line is expected", line_counter);
    }

    std::string value = what[1];

    if (value == "INCLUDE")
        tilt_specification = TiltSpecification::INCLUDE;
    else if (value == "NONE")
        tilt_specification = TiltSpecification::NONE;
    else
    {
        throw NotSupportedException(
            "TILT specification from file is not supported", line_counter);
    }
}


void IESParser::ProcessBlockKeywords(const std::string& keyword)
{
    if (keyword == "BLOCK" && ignore_blocks)
    {
        if (inside_block)
            throw ParsingException("BLOCK keyword is not expected", line_counter);
        inside_block = true;
    }
    else if (keyword == "ENDBLOCK" && ignore_blocks)
    {
        if (!inside_block)
            throw ParsingException("ENDBLOCK keyword is not expected", line_counter);
        inside_block = false;
    }
    else if (keyword == "ENDBLOCK" || keyword == "BLOCK")
    {
        throw NotSupportedException("Block support is not implemented", line_counter);
    }
}


void IESParser::AcceptKeyword(const std::string& keyword)
{
    if (keyword == "")
    {
        throw ParsingException("Keyword is empty", line_counter);
    }
    if (restrict_keyword_length || keyword.length() > MAX_KEYWORD_LENGTH)
    {
        throw ParsingException(
            "Keyword exceeds maximum length specified by IESNA standard", line_counter);
    }

    assert(format != Format::UNKNOWN);

    switch (format)
    {
    case Format::LM632002:
        if (!(keyword[0] == '_' || KeywordAllowedByIesna02(keyword)))
        {
            throw ParsingException(
                "Keyword " + keyword + " is not allowed by IESNA LM-63-2002 standard",
                line_counter);
        }
        break;
    case Format::LM631995:
        if (!(keyword[0] == '_' || KeywordAllowedByIesna95(keyword)))
        {
            throw ParsingException(
                "Keyword " + keyword + " is not allowed by IESNA LM-63-95 standard",
                line_counter);
        }
        break;
    case Format::LM631991:
        if (keyword[0] == '_')
        {
            throw ParsingException(
                "User keywords are not allowed by IESNA LM-63-91 standard",
                line_counter);
        }
        if (!KeywordAllowedByIesna91(keyword))
        {
            throw ParsingException(
                "Keyword " + keyword + " is not allowed by IESNA LM-63-91 standard",
                line_counter);
        }
        break;
    }
}


void IESParser::CheckEmpty(std::istream& input_stream, const std::string& line)
{
    if (!input_stream)
        throw ParsingException("End of file is not expectied", line_counter);
    if (line == "")
        throw ParsingException("Empty line is not expected", line_counter);
}