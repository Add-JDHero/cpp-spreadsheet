#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

class Sheet;

class Cell : public CellInterface {
    struct CellCache {
        Value value_;
        bool modify_ = true;

        operator bool() const;
        operator Value() const;
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
	
	mutable CellCache cache_;  
    
    bool IsModified() const;
    void SetCache(Value&& val) const;          
};

class Cell::Impl {
public:
	virtual ~Impl() = default;
	
	virtual Cell::Value GetValue(SheetInterface& sheet) const = 0;
	virtual std::string GetString() const = 0;
	virtual std::vector<Position> GetReferencedCells() const;
};

class Cell::EmptyImpl final : public Cell::Impl {
public:
	EmptyImpl() = default;
	
	Cell::Value GetValue([[maybe_unused]] SheetInterface& sheet) const override;    
	std::string GetString() const override;
};

class Cell::TextImpl final : public Cell::Impl {
public:
	explicit TextImpl(std::string text);
    
	Cell::Value GetValue([[maybe_unused]] SheetInterface& sheet) const override;    
	std::string GetString() const override;
private:
	const std::string data_;
};

class Cell::FormulaImpl final : public Cell::Impl {
    struct FormulaVisitor {
        Cell::Value operator() (double d) {
            return Cell::Value{d};
        }
        Cell::Value operator() (FormulaError fe) {
            return Cell::Value{fe};
        }
    };
public:
	explicit FormulaImpl(std::string formula);
	
	Cell::Value GetValue(SheetInterface& sheet) const override;
	std::string GetString() const override;
	std::vector<Position> GetReferencedCells() const override;
private:
	std::unique_ptr<FormulaInterface> data_;
};

struct ValueVisitor {
    Cell::Value operator() (std::string str) const;
    Cell::Value operator() (double d) const;
    Cell::Value operator() (FormulaError fe) const;
};

