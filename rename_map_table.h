#ifndef RENAME_MAP_TABLE_H   // Include guard to prevent multiple inclusions
#define RENAME_MAP_TABLE_H

#include <queue>
#include <string>

using namespace std;

/// @class RenameMapTable
/// @brief Class that contains an array of RMT elements by rob values and register indices.
class RenameMapTable
{
    public:
        RenameMapElement* renameMapTable;

        RenameMapTable()
        {
            renameMapTable = new RenameMapElement[size];
            for (int i = 0; i < size; ++i)
            {
                renameMapTable[i].Valid = false;
            }
        }
        
        /// @brief Adds or Updates a new element to the Rename Map table
        /// @param robIndex Reorder buffer value.
        /// @param registerIndex Register index/value.
        void AddElement(int robValue, int registerIndex)
        {
            renameMapTable[registerIndex].RegisterIndex = registerIndex;
            renameMapTable[registerIndex].RobValue = robValue;
            renameMapTable[registerIndex].Valid = true;
        }
        
        /// @brief Removes element at register index.
        /// @param registerIndex Register index/value.
        void RemoveElementAtIndex(int registerIndex)
        {
            renameMapTable[registerIndex].Valid = false;
        }
        
        /// @brief Gets whether the rename map table is empty or not based on the validity.
        bool IsEmpty()
        {
            for (int i = 0; i < size; i++)
            {
                if (renameMapTable[i].Valid)
                {
                    return false;
                }
            }
            return true;
        }

        /// @brief Attempts to get the element from the RMT.
        /// @param renameMapElement [out] Reference to an RMT element where the value will be stored if available.
        /// @return `true` if the element has a valid ROB value, `false` otherwise.
        bool TryGetElement(int registerIndex, RenameMapElement &renameMapElement)
        {
            if(IsEmpty())
            {
                return false;
            }
            for (int i = 0; i < size; i++)
            {
                if (renameMapTable[i].Valid && renameMapTable[i].RegisterIndex == registerIndex)
                {
                    renameMapElement = renameMapTable[registerIndex];
                    return true;
                }
            }
            return false;
        }
    
    private:
        unsigned long size = 67;
};

#endif