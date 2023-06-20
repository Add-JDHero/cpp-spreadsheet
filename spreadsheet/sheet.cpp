#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Position is not valid");
    }
    
    auto tmp_cell = std::move(std::make_unique<Cell>(*this));
    tmp_cell->Set(text);
    
    if (CheckForCircularDependencies(tmp_cell.get(), pos)) {
        throw CircularDependencyException("Circular dependency"s);
    }
    
    cells_[pos] = std::move(tmp_cell);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Position is not valid");
    }
    
    auto it = cells_.find(pos);
    return (it != cells_.end()) ? it->second.get() : nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Position is invalid!");
    }
    
    auto it = cells_.find(pos);
    return (it != cells_.end()) ? it->second.get() : nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Position is not valid");
    }
    
    cells_.erase(pos);
}

Size Sheet::GetPrintableSize() const {
    if (cells_.empty()) {
        return {0, 0};
    }
    
    auto left_right = GetLeftRightCorners();
    return {left_right.second.row - left_right.first.row + 1,
            left_right.second.col - left_right.first.col + 1};
}

void Sheet::PrintValues(std::ostream& output) const {
    auto size = GetPrintableSize();
    for (int i = 0; i < size.rows; ++i) {
        for (int j = 0; j < size.cols; ++j) {
            Position pos(i, j);
            auto cell_it = cells_.find(pos);
            if (cell_it != cells_.end()) {
                const auto& cellValue = cell_it->second->GetValue();
                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, FormulaError>) {
                        output << arg;
                    } else if constexpr (std::is_same_v<T, double>) {
                        output << arg;
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        output << arg;
                    }
                }, cellValue);
            } else {
                output << "";
            }
            if (j + 1 != size.cols) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < GetPrintableSize().rows; ++i) {
        for (int j = 0; j < GetPrintableSize().cols; ++j) {
            Position pos(i, j);
            auto cell = GetCell(pos);
            if (cell) {
                output << cell->GetText();
            }
            if (j < GetPrintableSize().cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

struct ValueToStringVisitor {
    std::string operator() (std::string str) const {
        return str;
    }
    std::string operator() (double d) const {
        std::ostringstream oss;
        oss << d;
        return oss.str();
    }
    std::string operator() (FormulaError fe) const {
        std::ostringstream oss;
        oss << fe;
        return oss.str();
    }
};

size_t Sheet::position_hash::operator() (const Position& p) const {
    return std::hash<int>()(p.row) ^ std::hash<int>()(p.col);
}

std::pair<Position, Position> Sheet::GetLeftRightCorners() const {
    if (cells_.empty()) {
        return {{0,0}, {0, 0}};
    } else if (cells_.size() == 1u) {
        auto pos = cells_.begin()->first;
        return { {0, 0}, pos };
    }
    
    Position right = Position::NONE;
    for (const auto& [key, _]: cells_) {
        right.col = right.col < key.col ? key.col : right.col;
        right.row = right.row < key.row ? key.row : right.row;
    }
    return { {0,0}, right };
}

bool Sheet::CheckForCircularDependencies(const CellInterface* cell, Position head) const {
    if(!cell) {
        return false;
    }
    bool res = false;
    for(const auto& next_cell_pos : cell->GetReferencedCells()) {
        if(next_cell_pos == head) {
            return true;
        }
        res = res || CheckForCircularDependencies(GetCell(next_cell_pos), head);
    }
    return res;
}