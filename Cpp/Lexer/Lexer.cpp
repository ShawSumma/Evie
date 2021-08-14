//
//
//
//  Made by Joonas Lehto at 10/5/2020 0:47AM
//  Few fixes by Gabe at 10/5/2020 0:47:00001AM
//
//
//
#include "../../H/Lexer/Lexer.h"
#include <stdexcept>
#include <sstream>
#include <optional>
#include <algorithm>
#include <fstream>
#include <cmath>
#include "../../H/UI/Safe.h"
#include "../../H/UI/Usr.h"

constexpr char LineEnding = '\n';
constexpr char BITWISE_XOR = '\xa4';

extern Usr* sys;

char Lexer::SingleLineCommentIdentifier = 0;
char Lexer::StringIdentifier = 0;
char Lexer::DecimalSeparator = 0;
char Lexer::ExponentSeparator = 0;

vector<string> Lexer::Operators;
vector<string> Lexer::Keywords;

enum class Type
{
    UNSPECIFIED,
    TEXT,
    COMMENT,
    NUMBER,
    PARENTHESIS,
    OPERATOR,
    STRING,
    END
};

struct Area
{
    Type Type = Type::UNSPECIFIED;

    string Text;

    Position Start;
    Position End;
};

template <typename T>
bool Exists(const vector<T> &source, const T &value)
{
    return find(source.begin(), source.end(), value) != source.end();
}

char GetParenthesisClosing(char opening, Position p)
{
    if (opening == '(')
    {
        return ')';
    }
    else if (opening == '[')
    {
        return ']';
    }
    else if (opening == '{')
    {
        return '}';
    }

    Report(Observation(ERROR, "Unrecognized parenthesis opening '" + opening + (string)"'", p));
}

/// <summary>
/// Returns all the characters which can mix with the specified character.
/// If this function returns null, it means the specified character can mix with any character.
/// </summary>
const char* GetMixingCharacters(char c)
{
    switch (c)
    {
        case '.': return ".0123456789";
        case ',': return "";
        case '<': return "=";
        case '>': return "=-";
        default: return nullptr;
    }
}

/// <summary>
/// Returns whether the two specified characters can mix
/// </summary>
bool Mixes(char a, char b)
{
    auto x = GetMixingCharacters(a);
    if (x != nullptr) return strchr(x, b) != nullptr;

    auto y = GetMixingCharacters(b);
    if (y != nullptr) return strchr(y, a) != nullptr;

    return true;
}

bool IsOperator(char c)
{
    return ((c >= 33 && c <= 47) || (c >= 58 && c <= 64) || c == '^' || c == '|' || c == BITWISE_XOR) && c != Lexer::SingleLineCommentIdentifier && c != Lexer::StringIdentifier ;
}

bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool IsText(char c)
{
    return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c == '_';
}

bool IsParenthesis(char c)
{
    return c == '(' || c == '[' || c == '{' || c == ')' || c == ']' || c == '}';
}

bool IsComment(char c)
{
    return c == Lexer::SingleLineCommentIdentifier;
}

bool IsString(char c)
{
    return c == Lexer::StringIdentifier;
}

Type GetType(char c)
{
    if (IsText(c))
    {
        return Type::TEXT;
    }
    else if (IsDigit(c))
    {
        return Type::NUMBER;
    }
    else if (IsParenthesis(c))
    {
        return Type::PARENTHESIS;
    }
    else if (IsOperator(c))
    {
        return Type::OPERATOR;
    }
    else if (IsComment(c))
    {
        return Type::COMMENT;
    }
    else if (IsString(c))
    {
        return Type::STRING;
    }
    else if (c == LineEnding)
    {
        return Type::END;
    }

    return Type::UNSPECIFIED;
}

bool IsPartOf(Type previous, Type current, char previous_symbol, char current_symbol)
{
    // Ensure the previous and the current character mix
    if (!Mixes(previous_symbol, current_symbol))
    {
        return false;
    }

    if (current == previous || previous == Type::UNSPECIFIED)
    {
        return true;
    }

    switch (previous)
    {

    case Type::TEXT:
    {
        return current == Type::NUMBER;
    }

    case Type::NUMBER:
    {
        return current_symbol == Lexer::DecimalSeparator ||                                                       // Example: 7.0
               current_symbol == Lexer::ExponentSeparator ||                                                      // Example: 100e0
               (previous_symbol == Lexer::ExponentSeparator && (current_symbol == '+' || current_symbol == '-')); // Examples: 3.14159e+10, 10e-10
    }

    default:
        return false;
    }
}

Position SkipSpaces(const string &text, Position &position)
{
    while (position.GetLocal() < text.size())
    {
        char c = text[position.GetLocal()];

        if (c != ' ')
        {
            break;
        }
        else
        {
            position.NextCharacter();
        }
    }

    return position;
}

Position SkipParenthesis(const string &text, const Position &start)
{
    Position position = start.Clone();

    char opening = text[position.GetLocal()];
    char closing = GetParenthesisClosing(opening, start);

    int count = 0;

    while (position.GetLocal() < text.size())
    {
        char c = text[position.GetLocal()];

        if (c == LineEnding)
        {
            position.NextLine();
        }
        else
        {
            if (c == opening)
            {
                count++;
            }
            else if (c == closing)
            {
                count--;
            }

            position.NextCharacter();
        }

        if (count == 0)
        {
            return position;
        }
    }

    MSG_Type Terminate = ERROR;

    if (sys->Info.Is_Service)
        Terminate = WARNING;

    Report(Observation(Terminate, "Couldn't find closing parenthesis", start));
}

Position SkipComment(const string &text, const Position &start)
{
    int i = (int)text.find(LineEnding, start.GetLocal());

    if (i != -1)
    {
        int length = i - start.GetLocal();
        return Position(start.GetLine(), start.GetCharacter() + length, start.GetLocal() + length, i).NextLine();
    }
    else
    {
        int length = (int)text.size() - start.GetLocal();
        return Position(start.GetLine(), start.GetCharacter() + length, start.GetLocal() + length, (int)text.size());
    }
}

Position SkipString(const string &text, const Position &start)
{
    int i = (int)text.find(Lexer::StringIdentifier, (size_t)start.GetLocal() + 1);
    int j = (int)text.find(LineEnding, (size_t)start.GetLocal() + 1);

    if (i == -1 || j != -1 && j < i)
    {
        MSG_Type Terminate = ERROR;

        if (sys->Info.Is_Service)
            Terminate = WARNING;

        Report(Observation(Terminate, "Couldn't find the end of the string", start));
    }

    int length = i + 1 - start.GetLocal();

    return Position(start.GetLine(), start.GetCharacter() + length, start.GetLocal() + length, i + 1);
}

optional<Area> GetNextComponent(const string &text, Position start)
{
    // Firsly the spaces must be skipped to find the next token
    Position position = SkipSpaces(text, start);

    // Verify there's text to iterate
    if (position.GetLocal() == text.size())
    {
        return nullopt;
    }

    Area area;
    area.Start = position.Clone();
    area.Type = GetType(text[position.GetLocal()]);

    switch (area.Type)
    {

    case Type::COMMENT:
    {
        area.End = SkipComment(text, area.Start);
        area.Text = text.substr(area.Start.GetLocal(), (size_t)area.End.GetLocal() - area.Start.GetLocal());
        return area;
    }

    case Type::PARENTHESIS:
    {
        area.End = SkipParenthesis(text, area.Start);
        area.Text = text.substr(area.Start.GetLocal(), (size_t)area.End.GetLocal() - area.Start.GetLocal());
        return area;
    }

    case Type::END:
    {
        area.End = position.Clone().NextLine();
        area.Text = "\n";
        return area;
    }

    case Type::STRING:
    {
        area.End = SkipString(text, area.Start);
        area.Text = text.substr(area.Start.GetLocal(), (size_t)area.End.GetLocal() - area.Start.GetLocal());
        return area;
    }

    default:
        break;
    }

    position.NextCharacter();

    // Possible types are now: TEXT, NUMBER, OPERATOR
    while (position.GetLocal() < text.size())
    {
        char current_symbol = text[position.GetLocal()];

        if (IsParenthesis(current_symbol))
        {
            // There cannot be number and content tokens side by side 1(
            /*if (area.Type == Type::NUMBER)
            {
                Report(Observation(ERROR, "Missing operator between number and parenthesis", position));
            }*/

            break;
        }

        Type type = GetType(current_symbol);
        char previous_symbol = position.GetLocal() == 0 ? (char)0 : text[(size_t)position.GetLocal() - 1];

        if (!IsPartOf(area.Type, type, previous_symbol, current_symbol))
        {
            break;
        }

        position.NextCharacter();
    }

    area.End = position;
    area.Text = text.substr(area.Start.GetLocal(), (size_t)area.End.GetLocal() - area.Start.GetLocal());

    return area;
}

Component ParseTextComponent(string text)
{
    if (Exists(Lexer::Operators, text))
    {
        return Component(text, Flags::OPERATOR_COMPONENT);
    }
    else if (Exists(Lexer::Keywords, text))
    {
        return Component(text, Flags::KEYWORD_COMPONENT);
    }
    else
    {
        return Component(text, Flags::TEXT_COMPONENT);
    }
}

optional<int> TryParseInt(string text)
{
    try
    {
        return stoi(text);
    }
    catch (...)
    {
        return nullopt;
    }
}

optional<int64_t> TryParseLong(string text)
{
    try
    {
        return stoll(text);
    }
    catch (...)
    {
        return nullopt;
    }
}

optional<double> TryParseDouble(string text)
{
    try
    {
        return stod(text);
    }
    catch (...)
    {
        return nullopt;
    }
}

string GetNumberPart(string text)
{
    int end = 0;

    for (; end < text.size() && (IsDigit(text[end]) || text[end] == Lexer::DecimalSeparator); end++);

    return text.substr(0, end);
}

int GetExponent(const string& text, Position p)
{
    int exponent_start = (int)text.find(Lexer::ExponentSeparator);

    if (exponent_start == -1)
    {
        return 0;
    }
    else
    {
        exponent_start++;

        int index = exponent_start;

        // Ensure that there's the exponent value
        if (index == text.size())
        {
            MSG_Type Terminate = ERROR;

            if (sys->Info.Is_Service)
                Terminate = WARNING;
            Report(Observation(Terminate,  "Invalid number exponent '" + text + "'", p));
        }

        // Skip the potential exponent sign
        if (text[(size_t)index + 1] == '+' || text[(size_t)index + 1] == '-')
        {
            index++;

            // Ensure that there's the exponent value
            if (index == text.size())
            {
                MSG_Type Terminate = ERROR;

                if (sys->Info.Is_Service)
                    Terminate = WARNING;
                Report(Observation(Terminate, "Invalid number exponent '" + text + "'", p));
            }
        }

        int exponent_end = index;

        // Get the exponent value's end index
        for (; exponent_end < text.size() && IsDigit(text[exponent_end]); exponent_end++);

        if (auto exponent = TryParseInt(text.substr(exponent_start, exponent_end)))
        {
            return exponent.value();
        }
        else
        {
            Report(Observation(ERROR, "Invalid number exponent '" + text + "'", p));
        }
    }
}

bool IsDecimal(string text)
{
    return text.find(Lexer::DecimalSeparator) != -1;
}

Component CreateNumberComponent(string text, const Position& position)
{
    int exponent = GetExponent(text, position);
    string number_part = GetNumberPart(text);

    if (IsDecimal(text))
    {
        if (auto number = TryParseDouble(number_part))
        {
            double value = number.value() * pow((long double)10.0, (long double)exponent);

            return Component(to_string(value), Flags::NUMBER_COMPONENT);
        }
        else
        {
            string s = "Invalid decimal number '" + text + "'";
            Report(Observation(ERROR, s, position));
        }
    }
    else
    {
        if (auto number = TryParseLong(number_part))
        {
            int64_t value = number.value() * (int64_t)pow(10.0, exponent);

            return Component(to_string(value), Flags::NUMBER_COMPONENT);
        }
        else
        {
            string s = "Invalid integer number '" + text + "'";

            Report(Observation(ERROR, s, position)); 
        }
    }
}

vector<Component> GetComponents(string text, Position anc);

Component CreateParenthesisComponent(string text, const Position& position)
{
    Component component(text, Flags::PAREHTHESIS_COMPONENT);
    component.Components = GetComponents(text.substr(1, text.size() - 2), position);

    return component;
}

Component ParseComponent(const Area& area)
{
    switch (area.Type)
    {
    case Type::TEXT:
        return ParseTextComponent(area.Text);
    case Type::NUMBER:
        return CreateNumberComponent(area.Text, area.Start);
    case Type::OPERATOR:
        return Component(area.Text, Flags::OPERATOR_COMPONENT);
    case Type::PARENTHESIS:
        return CreateParenthesisComponent(area.Text, area.Start);
    case Type::END:
        return Component("\n", Flags::END_COMPONENT);
    case Type::STRING:
        return Component(area.Text, Flags::STRING_COMPONENT);
    case Type::COMMENT:
        return Component(area.Text, Flags::COMMENT_COMPONENT);
    default:
    {
        string s = "Unrecognized token '" + area.Text + "'";

        Report(Observation(ERROR, s, area.Start));
    }

    }
}

vector<Component> GetComponents(string text, Position anchor)
{
    vector<Component> components;
    Position position(anchor.GetLine(), anchor.GetCharacter(), 0, anchor.GetAbsolute());

    replace(text.begin(), text.end(), '\t', ' ');
    replace(text.begin(), text.end(), '\r', ' ');

    while (position.GetLocal() < text.size())
    {
        optional<Area> area = GetNextComponent(text, position);

        if (!area)
        {
            break;
        }

        Component component = ParseComponent(area.value());
        component.Location = area->Start;
        components.push_back(component);
        
        position = area->End;
    }

    return components;
}

vector<Component> Lexer::GetComponents(string text)
{
    return GetComponents(text, Position());
}

Component Lexer::GetComponent(string text)
{
    Position position(0, 0, 0, 0);

    replace(text.begin(), text.end(), '\t', ' ');
    replace(text.begin(), text.end(), '\r', ' ');

    optional<Area> area = GetNextComponent(text, position);

    if (!area)
    {
        MSG_Type Terminate = ERROR;

        if (sys->Info.Is_Service)
            Terminate = WARNING;
        Report(Observation(Terminate, "Could not generate component from string '" + text + "'.", position));
    }

    Component component = ParseComponent(area.value());
    component.Location = area->Start;

    return component;
}

vector<Component> Lexer::GetComponentsFromFile(string file)
{
    if (Lexer::SingleLineCommentIdentifier == 0 || Lexer::StringIdentifier == 0 || Lexer::DecimalSeparator == 0 || Lexer::ExponentSeparator == 0)
    {
        Report(Observation(ERROR, "Please configure all the identifiers and separators needed by the lexer", Position()));
    }

    ifstream stream(file);

    if (!stream.is_open())
    {
        stringstream message;
        Report(Observation(ERROR, "Couldn't find or open file '" + file + "'", Position()));
    }

    string text;
    string line;

    while (getline(stream, line))
    {
        replace(line.begin(), line.end(), '\t', ' ');
        replace(text.begin(), text.end(), '\r', ' ');
        text += line + LineEnding;
    }

    stream.close();

    return GetComponents(text);
}