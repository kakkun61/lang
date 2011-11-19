#pragma once

/**
 * @param var 変数名
 * @param start 初期値
 * @param next 構造体の、次を指し示すメンバー
 */
#define GET_LAST(var, start, next) for ((var) = (start); (var)->next; (var) = (var)->next)

/**
 * @param var 変数名（構造体)
 * @param start 初期値
 * @param next 構造体の、次を指し示すメンバー
 */
#define FOR(var, start, next) for ((var) = (start); (var); (var) = (var)->next)

