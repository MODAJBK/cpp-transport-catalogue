#include "json.h"

using namespace std;

namespace json {

    namespace {

        //-------------------------LoadNode-----------------------

        Node LoadNode(istream& input);

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсинг целой части числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсинг дробной части числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсинг экспоненциальной части числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadBoolOrNull(std::istream& input) {
            std::string parsed_value;
            while (std::isalpha(input.peek())) {
                parsed_value += static_cast<char>(input.get());
            }
            if (parsed_value == "true"s) return Node(true);
            else if (parsed_value == "false"s) return Node(false);
            else if (parsed_value == "null"s) return Node(nullptr);
            else throw ParsingError("Error in parsing bool or null type");
        }


        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(s);
        }

        Node LoadArray(istream& input) {
            Array result;
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            if (it == end) {
                throw ParsingError("Parsing Error");
            }
            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            if (it == end) {
                throw ParsingError("Parsing Error");
            }
            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = Node(LoadString(input)).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'f' || c == 't' || c == 'n') {
                input.putback(c);
                return LoadBoolOrNull(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }// namespace

    //------------------------Node------------------------    

    Node::Node(std::nullptr_t value)
        : value_(value) {
    }

    Node::Node(int value)
        : value_(value) {
    }

    Node::Node(double value)
        : value_(value) {
    }

    Node::Node(bool value)
        : value_(value) {
    }

    Node::Node(std::string value)
        :value_(std::move(value)) {
    }

    Node::Node(Array value)
        : value_(std::move(value)) {
    }

    Node::Node(Dict value)
        : value_(std::move(value)) {
    }

    bool Node::operator==(const Node& rs) const {
        return value_ == rs.value_;
    }

    bool Node::operator!=(const Node& rs) const {
        return !(*this == rs);
    }

    Node::Value Node::GetValue() const {
        return value_;
    }

    //------------------------Node::IsType()------------------------

    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    //------------------------Node::AsType()------------------------    

    int Node::AsInt() const {
        if (!IsInt()) throw std::logic_error("Value type is not int"s);
        return std::get<int>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) throw std::logic_error("Value type is not bool"s);
        return std::get<bool>(value_);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) throw std::logic_error("Value type is not int or double"s);
        if (IsInt()) return static_cast<double>(std::get<int>(value_));
        return std::get<double>(value_);
    }

    const std::string& Node::AsString() const {
        if (!IsString()) throw std::logic_error("Value type is not std::string"s);
        return std::get<std::string>(value_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) throw std::logic_error("Value type is not Array"s);
        return std::get<Array>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) throw std::logic_error("Value type is not Array"s);
        return std::get<Dict>(value_);
    }

    //-----------------------PrintValue------------------------
    /*
    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out) {
        out << value;
    }*/

    void PrintValue(int value, std::ostream& out) {
        out << value;
    }

    void PrintValue(double value, std::ostream& out) {
        out << value;
    }

    void PrintValue(std::nullptr_t, std::ostream& out) {
        out << "null"sv;
    }

    void PrintValue(bool value, std::ostream& out) {
        out << std::boolalpha << value;
    }

    void PrintValue(const std::string& value, std::ostream& out) {
        out << "\""s;
        for (char ch : value) {
            switch (ch) {
            case('\"'):
                out << "\\\""s;
                break;
            case('\\'):
                out << "\\\\"s;
                break;
            case('\n'):
                out << "\\n"s;
                break;
            case('\r'):
                out << "\\r"s;
                break;
            default:
                out << ch;
            }
        }
        out << "\""s;
    }

    void PrintValue(const Array&, std::ostream&);

    void PrintValue(const Dict& value, std::ostream& out) {
        out << '{';
        for (const auto& node : value) {
            PrintValue(node.first, out);
            out << ':';
            if (node.second.IsArray()) PrintValue(node.second.AsArray(), out);
            else if (node.second.IsMap()) PrintValue(node.second.AsMap(), out);
            else if (node.second.IsInt()) PrintValue(node.second.AsInt(), out);
            else if (node.second.IsPureDouble()) PrintValue(node.second.AsDouble(), out);
            else if (node.second.IsBool()) PrintValue(node.second.AsBool(), out);
            else if (node.second.IsString()) PrintValue(node.second.AsString(), out);
            else if (node.second.IsNull()) PrintValue(nullptr, out);
            if (node != *value.rbegin()) out << ',';
        }
        out << '}';
    }

    void PrintValue(const Array& value, std::ostream& out) {
        size_t size = value.size();
        out << '[';
        for (size_t index = 0; index < size; ++index) {
            if (value[index].IsArray()) PrintValue(value[index].AsArray(), out);
            else if (value[index].IsMap()) PrintValue(value[index].AsMap(), out);
            else if (value[index].IsInt()) PrintValue(value[index].AsInt(), out);
            else if (value[index].IsPureDouble()) PrintValue(value[index].AsDouble(), out);
            else if (value[index].IsBool()) PrintValue(value[index].AsBool(), out);
            else if (value[index].IsString()) PrintValue(value[index].AsString(), out);
            else if (value[index].IsNull()) PrintValue(nullptr, out);
            if (index != size - 1) out << ',';
        }
        out << ']';
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
            [&out](const auto& value) { PrintValue(value, out); },
            node.GetValue());
    }

    //-----------------------Document------------------------

    Document::Document(Node root)
        : root_(move(root)) {
    }

    bool Document::operator==(const Document& rs) const {
        return root_ == rs.root_;
    }

    bool Document::operator!=(const Document& rs) const {
        return !(*this == rs);
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

}  // namespace json