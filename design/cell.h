#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
	struct CellCashe {
	  Value value_;
	  bool modify_ = true;
	};

public:
	Cell(Sheet& sheet);
	~Cell();

	void Set(std::string text);
	void Clear();

	Value GetValue() const override;
	std::string GetText() const override;

	std::vector<Position> GetReferencedCells() const override;
private:
	class Impl;
	class EmptyImpl;
	class TextImpl;
	class FormulaImpl;

	std::unique_ptr<Impl> impl_;
	Sheet& sheet_;
	CellCashe cashe_;
};
