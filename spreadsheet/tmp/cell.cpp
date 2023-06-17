#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

#include "formula.h"

class Cell::Impl {
public:
    virtual ~Impl() = default;

    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
};

class Cell::TextImpl : public Cell::Impl {
public:
    explicit TextImpl(std::string text)
        : text_(std::move(text)) {
    }

    CellInterface::Value GetValue() const override {
        return text_.at(0) == ESCAPE_SIGN ? text_.substr(1) : text_;
    }

    std::string GetText() const override {
        return text_;
    }

private:
    std::string text_;
};

class Cell::EmptyImpl final : public Impl {
public:
    EmptyImpl() = default;

    CellInterface::Value GetValue() const override {
        return std::string("");
    }

    std::string GetText() const override {
        return std::string("");
    }
};

class Cell::FormulaImpl : public Cell::Impl {
public:
    explicit FormulaImpl(std::string expression)
        : formula_(ParseFormula(std::move(expression))) {
    }

    CellInterface::Value GetValue() const override {
        try {
            auto result = formula_->Evaluate();
            if (std::holds_alternative<double>(result)) {
                return std::get<double>(result);
            } else if(std::holds_alternative<FormulaError>(result)) {
                return std::get<FormulaError>(result);
            }
        } catch (const FormulaException& e) {
            return FormulaError{"Formula exception encountered"};
        }
        return FormulaError{"Invalid value"};
    }

    std::string GetText() const override {
        return FORMULA_SIGN + formula_->GetExpression();
    }

private:
    std::unique_ptr<FormulaInterface> formula_;
};

Cell::Cell()
    : impl_(std::make_unique<TextImpl>("")) {
}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
        return;
    } else if (text.size() > 1 && text.at(0) == FORMULA_SIGN) {
        impl_ = std::make_unique<FormulaImpl>(text.substr(1));
        return;
    }
    
    impl_ = std::make_unique<TextImpl>(text);
}


Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}