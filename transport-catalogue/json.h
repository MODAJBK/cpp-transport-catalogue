#pragma once

#include <iostream>
#include <map>
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    // ��������� ���������� Dict � Array ��� ���������
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // ��� ������ ������ ������������� ��� ������� �������� JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    // �������� ������, ������ ������ �� ����� ������ � ������� �����
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // ���������� ����� �������� ������ � ����������� ���������
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    class Node final :private std::variant<std::nullptr_t, bool, int, double, std::string, Array, Dict> {
    public:

        using variant::variant;
        using Value = variant;

        Node(Value value);

        bool operator==(const Node& rs) const;
        bool operator!=(const Node& rs) const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        const Value& GetValue() const;
        Value& GetValue();

    };

    class Document {
    public:
        explicit Document(Node root);

        bool operator==(const Document& rs) const;
        bool operator!=(const Document& rs) const;

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json#pragma once
