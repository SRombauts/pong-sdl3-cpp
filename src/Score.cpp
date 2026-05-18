#include "Score.h"

#include <string>

void setScore(Score& score, int newValue)
{
    score.value = newValue;
    score.text = std::to_string(newValue);
}
