#include <iostream>
#include <sstream>
#include <vector>

#include "element.hpp"

using namespace std;

namespace mongo {

    Document::Document() : BContainer()  { }

    Document::Document(const bson_t bson) : BContainer(bson) {

    }

    Document::Document(const Document& rhs) : BContainer(rhs) {
        elements_ = rhs.elements_;
    }

    Document::~Document() { }

    std::ostream& operator<< (std::ostream& os, Document d) {
        os << d.toString();
        return os;
    }

    std::string Document::toString() {
        // static int indent = 0;
        std::stringstream s;
        // s << "{ " << endl;
        // indent++;
        // string prefix(indent * 2, ' ');
        // for (auto e : elements_)
        //     s << prefix << e.first << ": " << e.second << std::endl;
        // indent--;
        // string suffix(indent * 2, ' ');
        // s << suffix << "} ";
        size_t len;
        char* json = bson_as_json(&bson_, &len);
        return std::string(json, len);
    }

    Element& Document::operator[](const char* field) {
        // return existing field's value
        auto e = elements_.find(field);
        if (e != elements_.end())
            return *e->second;

        // set a new value
        std::shared_ptr<Element> val(new Element(this));
        val->setFieldName(field);
        elements_.insert(make_pair(field, val));
        return *val;
    }

}

