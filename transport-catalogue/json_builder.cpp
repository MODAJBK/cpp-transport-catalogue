#include "json_builder.h"

using namespace std::string_literals;

namespace json {

//----------------------Builder----------------------

    Builder::DictItemsContext Builder::StartDict() {
        DictItemsContext result(*this);
        if (nodes_stack_.empty() || root_.IsNull()) {
            root_ = Node(Dict{});
            nodes_stack_.emplace_back(&root_);
        }
        else if (nodes_stack_.back()->IsNull()) {
            *nodes_stack_.back() = Node(Dict{});
        }
        else if (nodes_stack_.back()->IsArray()) {
            Array& started_array = std::get<Array>(nodes_stack_.back()->GetValue());
            started_array.emplace_back(std::move(Dict{}));
            nodes_stack_.emplace_back(&started_array.back());
        }
        else {
            throw std::logic_error("No elements to build"s);
        }
        return result;
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("There are no building elements"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder::DictKeyContext Builder::Key(std::string key) {
        if (nodes_stack_.empty() || root_.IsNull()) {
            throw std::logic_error("Error, Node is alredy constructed"s);
        }
        DictKeyContext result(*this);
        Dict& dict = std::get<Dict>(nodes_stack_.back()->GetValue());
        dict.emplace(key, Node{});
        nodes_stack_.emplace_back(&dict.at(key));
        return result;
    }

    Builder::ArrayItemsContext Builder::StartArray() {
        ArrayItemsContext result(*this);
        if (nodes_stack_.empty() || root_.IsNull()) {
            root_ = Node(Array{});
            nodes_stack_.emplace_back(&root_);
        }
        else if (nodes_stack_.back()->IsNull()) {
            *nodes_stack_.back() = Node(Array{});
        }
        else if (nodes_stack_.back()->IsArray()) {
            Array& started_array = std::get<Array>(nodes_stack_.back()->GetValue());
            started_array.emplace_back(std::move(Array{}));
            nodes_stack_.emplace_back(&started_array.back());
        }
        else {
            throw std::logic_error("No elements to build"s);
        }
        return result;
    }

    Builder& Builder::EndArray() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("There are no building elements"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::Value(Node::Value value) {
        if (root_.IsNull() && nodes_stack_.empty()) {
            root_ = value;
        }
        else if (!nodes_stack_.empty() && nodes_stack_.back()->IsNull()) {
            *nodes_stack_.back() = value;
            nodes_stack_.pop_back();
        }
        else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            Array& array = std::get<Array>(nodes_stack_.back()->GetValue());
            array.emplace_back(std::move(value));
        }
        else {
            throw std::logic_error("Error, Node is alredy constructed"s);
        }
        return *this;
    }

    Node Builder::Build() {
        if (root_.IsNull() || !nodes_stack_.empty()) {
            throw std::logic_error("Node construation is not finished yet"s);
        }
        return root_;
    }

//----------------------BaseContext----------------------

    Builder::BaseContext::BaseContext(Builder& builder)
        : builder_(builder)
    {}

    Builder::DictItemsContext Builder::BaseContext::StartDict() {
        return builder_.StartDict();
    }

    Builder& Builder::BaseContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::DictKeyContext Builder::BaseContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    Builder::ArrayItemsContext Builder::BaseContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& Builder::BaseContext::EndArray() {
        return builder_.EndArray();
    }

    Builder& Builder::BaseContext::Value(Node::Value value) {
        return builder_.Value(std::move(value));
    }

    Node Builder::BaseContext::Build() {
        return builder_.Build();
    }

//----------------------OtherContexts----------------------

    Builder::DictItemsContext::DictItemsContext(Builder& builder) 
        : BaseContext(builder)
    {}

    Builder::DictKeyContext::DictKeyContext(Builder& builder) 
        : BaseContext(builder)
    {}

    Builder::DictItemsContext Builder::DictKeyContext::Value(Node::Value value) {
        return Builder::DictItemsContext(BaseContext::Value(std::move(value)));
    }

    Builder::ArrayItemsContext::ArrayItemsContext(Builder& builder) 
        : BaseContext(builder)
    {}

    Builder::ArrayItemsContext& Builder::ArrayItemsContext::Value(Node::Value value) {
        BaseContext::Value(std::move(value));
        return *this;
    }

}