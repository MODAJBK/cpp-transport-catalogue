#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
        switch (line_cap)
        {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
        switch (line_join)
        {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        }
        return out;
    }

    Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b)
        : red(r)
        , green(g)
        , blue(b)
    {}

    std::ostream& operator<<(std::ostream& out, const Rgb& color) {
        out << "rgb("sv << static_cast<int>(color.red) << ','
            << static_cast<int>(color.green) << ','
            << static_cast<int>(color.blue) << ')';
        return out;
    }

    Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double op)
        : red(r)
        , green(g)
        , blue(b)
        , opacity(op)
    {}

    std::ostream& operator<<(std::ostream& out, const Rgba& color) {
        out << "rgba("sv << static_cast<int>(color.red) << ','
            << static_cast<int>(color.green) << ','
            << static_cast<int>(color.blue) << ','
            << color.opacity << ')';
        return out;
    }

    // ---------- ColorPrinter ------------------

    void ColorPrinter::operator()(std::monostate) const {
        out << "none"sv;
    }

    void ColorPrinter::operator()(std::string color) const {
        out << color;
    }

    void ColorPrinter::operator()(Rgb color) const {
        out << color;
    }

    void ColorPrinter::operator()(Rgba color) const {
        out << color;
    }

    std::ostream& operator<<(std::ostream& out, Color color) {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }

    // ---------- Point ------------------

    bool Point::operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool Point::operator!=(const Point& other) const {
        return !(*this == other);
    }

    // ---------- Object ------------------

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); ++i) {
            out << points_[i].x << ',' << points_[i].y;
            if (i == points_.size() - 1) break;
            out << ' ';
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }


    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        text_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text ";
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y
            << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y
            << "\" font-size=\""sv << size_ << '\"';
        if (!family_.empty()) out << " font-family=\""sv << family_ << '\"';
        if (!weight_.empty()) out << " font-weight=\""sv << weight_ << '\"';
        out << '>';
        for (auto ch : text_) {
            if (ch == '\"') out << "&quot;"sv;
            else if (ch == '\'') out << "&apos;"sv;
            else if (ch == '<') out << "&lt;"sv;
            else if (ch == '>') out << "&gt;"sv;
            else if (ch == '&') out << "&amp;"sv;
            else out << ch;
        }
        out << "</text>"sv;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        RenderContext rnd_context(out);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << '\n'
            << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << '\n';
        for (size_t i = 0; i < objects_.size(); ++i) {
            out << ' ';
            objects_[i]->Render(rnd_context);
        }
        out << "</svg>"sv;
    }

}  // namespace svg