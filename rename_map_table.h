#ifndef RENAME_MAP_TABLE_H   // Include guard to prevent multiple inclusions
#define RENAME_MAP_TABLE_H

#include <queue>
#include <string>

using namespace std;

struct RenameMapElement
{
    int RegisterIndex;
    int RobIndex;
    bool Valid;
};

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
        
        void AddElement(int isValid, int robIndex, int registerIndex)
        {
            renameMapTable[registerIndex].RegisterIndex = registerIndex;
            renameMapTable[registerIndex].RobIndex = robIndex;
            renameMapTable[registerIndex].Valid = isValid;
        }
        
        void RemoveElement(int registerIndex)
        {
            renameMapTable[registerIndex].Valid = false;
        }

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