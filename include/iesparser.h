// Author: Artem Bishev

#ifndef IESPARSER_H
#define IESPARSER_H

#include <iostream>
#include <string>
#include <unordered_map>


class IESParser
{
    // Exception that stores erroneous line number.
    class Exception : public std::exception
    {
    public:

        Exception(const char* message, int line) :
            msg(message), line_number(line)
        {}

        Exception(const std::string& message, int line) :
            msg(message), line_number(line)
        {}

        virtual ~Exception() throw (){}

        virtual const char* what() const throw () override
        {
            return msg.c_str();
        }

        virtual int line() const throw ()
        {
            return line_number;
        }

    protected:
        std::string msg;
        int line_number;
    };

    // File format violates the IES specifications.
    class ParsingException : public Exception
    {
    public:
        ParsingException(const char* message, int line) :
            Exception(message, line)
        {}

        ParsingException(const std::string& message, int line) :
            Exception(message, line)
        {}
    };

    // Feature is not properly supported by this parser.
    class NotSupportedException : public Exception
    {
    public:
        NotSupportedException(const char* message, int line) :
            Exception(message, line)
        {}

        NotSupportedException(const std::string& message, int line) :
            Exception(message, line)
        {}
    };

    typedef std::unordered_map<std::string, std::string> KeywordsDictionary;

    enum Format
    {
        UNKNOWN,
        LM_63_1986,
        LM_63_1991,
        LM_63_1995,
        LM_63_2002,
    };

    enum TiltSpecification
    {
        INCLUDE,
        FILE,
        NONE
    };

    // Parse input stream containing IESNA LM-63 Photometric Data.
    void Parse(std::istream& input_stream);

    // Check if the keyword is allowed by IESNA LM-63-2002 standard.
    static bool KeywordAllowedByIesna02(const std::string& keyword);

    // Check if the keyword is allowed by IESNA LM-63-95 standard.
    static bool KeywordAllowedByIesna95(const std::string& keyword);

    // Check if the keyword is allowed by IESNA LM-63-91 standard.
    static bool KeywordAllowedByIesna91(const std::string& keyword);

    // Options:
    bool restrict_keyword_length = false;
    bool ignore_allowed_keywords = false;
    bool ignore_required_keywords = false;
    bool ignore_blocks = false;
    bool ignore_empty_lines = true;

private:
    const char* const KEYWORD_LINE_REGEX = "(.*)[:space:]+(\[\w*\])";
    const char* const TILT_LINE_REGEX = "TILT[:space:]*=[:space:]*(.*)";

    static const int MAX_KEYWORD_LENGTH = 18;

    // Read line and increase counter.
    std::string ReadLine(std::istream& input_stream) const;

    // Read line, trim it and increase counter.
    // When ignore_empty_lines set to true this method
    // ignores all lines consisting of whitespace characters.
    std::string ReadTrimmedLine(std::istream& input_stream) const;

    // Retrieve format version.
    // UNKNOWN is set if this line is not one of supported version strings.
    void ParseFormatVersion(const std::string& version_string);

    bool IsKeywordLine(const std::string& line) const;

    bool IsTiltLine(const std::string& line) const;

    void ParseKeywordLine(const std::string& line);

    void ParseTiltLine(const std::string& line);

    // Process BLOCK and ENDBLOCK keywords.
    void ProcessBlockKeywords(const std::string& keyword);

    // Check if the specified standard allows this keyword.
    void AcceptKeyword(const std::string& keyword);

    // Check if all required keywords were found.
    void CheckRequiredKeywords();

    void CheckIesna02RequiredKeywords();

    void CheckIesna91RequiredKeywords();

    // Check if line is not empty and EOF is not reached.
    void CheckEmpty(std::istream& input_stream, const std::string& line);

    Format format;
    TiltSpecification tilt_specification;
    std::string tilt_specification_filename;
    KeywordsDictionary keywords_dictionary;
    KeywordsDictionary::iterator last_added_keyword;
    mutable int line_counter;
    bool inside_block = false;
};

#endif // IESPARSER_H
