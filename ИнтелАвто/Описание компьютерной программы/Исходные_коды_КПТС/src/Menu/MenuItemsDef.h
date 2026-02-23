// (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "Menu/MenuItems.h"


inline void FuncVV() { }

#define DEF_BUTTON(name, titleRu, funcPress)                                                                        \
static Button name(titleRu, funcPress)

#define DEF_CHOICE_2(name, nameRu1, nameRu2, states, func)                                                          \
static pchar n##name##Ru[] = { nameRu1, nameRu2, nullptr };                                                         \
static Choice name(nullptr, n##name##Ru, func, ((uint8 *)&states));


#define DEF_CHOICE_1_FULL(name, title, nameRu1, states, func)                                              \
static pchar n##name##Ru[] = { nameRu1, nullptr };                                                         \
static Choice name(title, n##name##Ru, func, ((uint8 *)&states));


#define DEF_CHOICE_2_FULL(name, title, nameRu1, nameRu2, states, func)                                              \
static pchar n##name##Ru[] = { nameRu1, nameRu2, nullptr };                                                         \
static Choice name(title, n##name##Ru, func, ((uint8 *)&states));


#define DEF_CHOICE_4_FULL(name, title, nameRu1, nameRu2, nameRu3, nameRu4, states, func)                            \
static pchar n##name##Ru[] = { nameRu1, nameRu2, nameRu3, nameRu4, nullptr };                                       \
static Choice name(title, n##name##Ru, func, ((uint8 *)&states));
