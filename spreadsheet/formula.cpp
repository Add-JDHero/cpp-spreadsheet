#include "formula.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

Formula::Formula(std::string expression) 
    : ast_(ParseFormulaAST(std::move(expression)))
{
    auto cells = ast_.GetCells();
    cells.unique();
    cells.sort();
    cells_ = { cells.begin(), cells.end() };
}

Formula::Value Formula::Evaluate(const SheetInterface& arg) const {
    try {
        return ast_.Execute(arg);
    } catch (const FormulaError& e) {
        return e;
    }
}

std::string Formula::GetExpression() const {
    std::ostringstream os;
    ast_.PrintFormula(os);
    return os.str();
}
    
std::vector<Position> Formula::GetReferencedCells() const {
    return cells_;
}
    
std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch(...) {
        throw FormulaException{"Unable to parse: "s.append(expression)};
    }
}