#pragma once

#include <utility>
#include <exception>
#include <optional>
#include <vector>

#include "json.h"

namespace json {

    class Builder {

        class BaseContext;
        class DictItemsContext;
        class DictKeyContext;
        class ArrayItemsContext;

    public:

        Builder() = default;

        DictItemsContext StartDict();
        Builder& EndDict();
        DictKeyContext Key(std::string key);

        ArrayItemsContext StartArray();
        Builder& EndArray();

        Builder& Value(Node::Value value);
        
        Node Build();

    private:

        Node root_;
        std::vector<Node*> nodes_stack_;

        class BaseContext {
        public:

            BaseContext(Builder&);

            DictItemsContext StartDict();
            Builder& EndDict();
            DictKeyContext Key(std::string key);

            ArrayItemsContext StartArray();
            Builder& EndArray();

            Builder& Value(Node::Value value);

            Node Build();

        private:

            Builder& builder_;

        };


        class DictItemsContext final : public Builder::BaseContext {
        public:

            DictItemsContext(Builder&);

            DictItemsContext StartDict() = delete;
            ArrayItemsContext StartArray() = delete;
            Builder& EndArray() = delete;
            Builder& Value(Node::Value) = delete;
            Node Build() = delete;

        };

        class DictKeyContext final : public Builder::BaseContext {
        public:

            DictKeyContext(Builder&);

            DictItemsContext Value(Node::Value value);

            DictKeyContext Key(std::string key) = delete;
            Builder& EndDict() = delete;
            Builder& EndArray() = delete;
            Node Build() = delete;

        };

        class ArrayItemsContext final : public Builder::BaseContext {
        public:

            ArrayItemsContext(Builder&);

            ArrayItemsContext& Value(Node::Value value);

            DictKeyContext Key(std::string key) = delete;
            Builder& EndDict() = delete;
            Node Build() = delete;

        };

    };

}