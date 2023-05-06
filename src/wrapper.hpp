/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   wrapper.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrosa-do <lrosa-do@student.42lisboa>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/17 07:09:53 by lrosa-do          #+#    #+#             */
/*   Updated: 2023/04/20 17:23:45 by lrosa-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Engine.hpp"


void LoadLua();
void CloseLua();
bool ReloadLuaScript(const char *filename);
void InitEngine();
void EngineRender();

double getLuaMemoryUsage();
void Collect();
lua_State *getState();
std::size_t GetAllocatedMemory();
void addMemory(std::size_t size);
void removeMemory(std::size_t size);
void PrintLuaStack(lua_State *L);
void PrintLuaTable(lua_State *L);
void PrintTopValue(lua_State *L);