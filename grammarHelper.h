#ifndef GRAMMAR_HELPER
#define GRAMMAR_HELPER

#define NUMBER_BASE 10

enum StateAction {Nothing, WriteTerminate, WriteChar, WriteWord, WriteHalf, ReadComma, WriteByte, SetExternSymbol, SetEntrySymbol, 
                    InstructionParse, AddCodeSymbol, AddDataSymbol, PrintWarn, NullPrevious, SavePosition};

enum {StateError = -2, StateAccept = -1};

/* Returns the action the current state requires be run */
enum StateAction getStateAction(int currentState);

/* Matches the conditions and returns the index of the next state, given the current state, and pointer to input */
int getNextState(int currentState, char **p_line);

/* Returns the defined error message for a state */
char *getStateErrorMessage(int currentState);

int Spacing(char **p_line);
int IsAlnum(char **p_line);

#endif
