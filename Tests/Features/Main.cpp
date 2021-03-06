﻿/*
The Jinx library is distributed under the MIT License (MIT)
https://opensource.org/licenses/MIT
See LICENSE.TXT or Jinx.h for license details.
Copyright (c) 2016 James Boer
*/

#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../../Source/Jinx.h"

using namespace Jinx;

#define REQUIRE assert

Jinx::RuntimePtr TestCreateRuntime()
{
	return Jinx::CreateRuntime();
}

Jinx::ScriptPtr TestCreateScript(const char * scriptText, Jinx::RuntimePtr runtime = nullptr)
{
	if (!runtime)
		runtime = CreateRuntime();

	// Compile the text to bytecode
	auto bytecode = runtime->Compile(scriptText, "Test Script", { "core" });
	if (!bytecode)
		return nullptr;

	// Create a runtime script with the given bytecode
	return runtime->CreateScript(bytecode);
}

Jinx::ScriptPtr TestExecuteScript(const char * scriptText, Jinx::RuntimePtr runtime = nullptr)
{
	// Create a runtime script 
	auto script = TestCreateScript(scriptText, runtime);
	if (!script)
		return nullptr;

	// Execute script until finished
	do
	{
		if (!script->Execute())
			return nullptr;
	} 
	while (!script->IsFinished());

	return script;
}

int main(int argc, char ** argv)
{
	printf("Jinx version: %s\n", Jinx::GetVersionString().c_str());

	Initialize(GlobalParams());
	// Scope block to ensure all objects are destroyed for shutdown test
	{
		const char * scriptText =
			u8R"(

				function calculate volume from width: {w} height: {h} depth: {d}
					return w * h * d
				end

				set x to calculate volume from width: 3 height: 4 depth: 5

			)";

		auto script = TestExecuteScript(scriptText);
		REQUIRE(script->GetVariable("x") == 60);
	}
	ShutDown();
    return 0;
}