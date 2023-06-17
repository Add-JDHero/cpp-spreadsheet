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
    
    if (cells_.count(pos) == 0) {
        cells_[pos] = std::make_unique<Cell>();
    }
    cells_[pos]->Set(std::move(text));
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
    int max_row = 0;
    int max_col = 0;

    for (const auto& [pos, cell] : cells_) {
        max_row = std::max(max_row, pos.row + 1);
        max_col = std::max(max_col, pos.col + 1);
    }

    return {max_row, max_col};
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