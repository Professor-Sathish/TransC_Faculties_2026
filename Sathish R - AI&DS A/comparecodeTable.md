# C Program Enhancement Comparison Table

## Overview
This table compares the source code (paste-2.txt) with the enhanced version (paste.txt) of a bank account management program in C.

## Feature Comparison

| **Aspect** | **Source Code** | **Enhanced Code** | **Enhancement Details** |
|------------|----------------|-------------------|------------------------|
| **Sorting Options** | Single sorting by balance only (highest to lowest) | Multiple sorting criteria with flexible ordering | Added 4 sorting options: balance, name, max balance, min balance with ascending/descending choice |
| **Sort Algorithm** | Bubble sort implementation | qsort() with custom comparators | Replaced inefficient bubble sort with standard library's optimized qsort function |
| **Function Structure** | Single `sortByBalance()` function | Modular approach with `sortOption()` and `sortAccounts()` | Better separation of concerns - UI logic separated from sorting logic |
| **User Interface** | No sorting order choice | Interactive menu for sorting criteria and order | Added sub-menus for selecting sort type and ascending/descending order |
| **Comparison Functions** | Inline comparison in bubble sort | Dedicated comparator functions | Added `compareByBalance()` and `compareByName()` functions for cleaner code |
| **Data Validation** | Basic account number check | Comprehensive data validation | Added checks for account number range (1-100) and balance limits (-1M to 10M) |
| **String Sanitization** | No string cleaning | `sanitizeString()` function | Added function to remove non-printable characters from names |
| **Global Variables** | None | `ascending_order` global variable | Added to communicate sort order to comparator functions |
| **Name Sorting** | Not available | Full name sorting (last name, then first name) | Enhanced to sort by last name primarily, first name secondarily |
| **Min/Max Features** | Not available | Dedicated min/max balance display | Added options to find and display accounts with minimum/maximum balances |
| **Error Handling** | Basic "no accounts" scenario | Enhanced validation and error messages | Better handling of edge cases and invalid data |
| **Code Organization** | Linear function flow | Structured with clear function separation | Better modularity and maintainability |

## Detailed Function Comparison

| **Function** | **Source Code** | **Enhanced Code** | **Key Differences** |
|--------------|----------------|-------------------|-------------------|
| **Main Sorting Function** | `sortByBalance()` - 47 lines | `sortAccounts()` - 82 lines | More comprehensive with multiple criteria support |
| **UI Handling** | Direct call to sort function | `sortOption()` function handles user input | Separated user interface from sorting logic |
| **Comparison Logic** | Inline if-else statements | Dedicated `compareByBalance()` and `compareByName()` functions | Cleaner, reusable comparison logic |
| **Data Processing** | Simple valid account extraction | Advanced validation with sanitization | More robust data handling |

## Technical Improvements

| **Category** | **Source Enhancement** | **Impact** |
|--------------|----------------------|------------|
| **Performance** | Bubble Sort O(n²) → qsort O(n log n) | Significantly faster for larger datasets |
| **Maintainability** | Monolithic function → Modular design | Easier to modify and extend |
| **User Experience** | Fixed sorting → Interactive menu system | More flexible and user-friendly |
| **Data Integrity** | Basic validation → Comprehensive sanitization | Better handling of corrupted or invalid data |
| **Code Quality** | Procedural approach → Function-based design | Better adherence to software engineering principles |

## New Features Added

1. **Multiple Sorting Criteria**
   - Sort by balance (ascending/descending)
   - Sort by name (alphabetical)
   - Find maximum balance account
   - Find minimum balance account

2. **Data Sanitization**
   - `sanitizeString()` function removes non-printable characters
   - Prevents display issues with corrupted data

3. **Enhanced Validation**
   - Account number range validation (1-100)
   - Balance range validation (-1,000,000 to 10,000,000)
   - Comprehensive data integrity checks

4. **Improved User Interface**
   - Interactive menus for sorting options
   - Clear prompts and formatted output
   - Better error messages and user guidance

## Code Quality Metrics

| **Metric** | **Source Code** | **Enhanced Code** | **Improvement** |
|------------|----------------|-------------------|-----------------|
| **Lines of Code** | ~180 lines | ~320 lines | +78% (added functionality) |
| **Functions** | 6 functions | 9 functions | +50% (better modularity) |
| **Complexity** | Medium | Higher but well-organized | Better structured complexity |
| **Reusability** | Limited | High | Functions designed for reuse |
| **Maintainability** | Moderate | High | Clear separation of concerns |

## Summary

The enhanced code represents a significant improvement over the source code, transforming a simple balance-sorting feature into a comprehensive account management system with multiple sorting options, better data validation, and improved user experience. The changes demonstrate good software engineering practices including modularity, data validation, and user-centered design.