﻿/*
The Jinx library is distributed under the MIT License (MIT)
https://opensource.org/licenses/MIT
See LICENSE.TXT or Jinx.h for license details.
Copyright (c) 2016 James Boer
*/

#include <assert.h>
#include <array>
#include <thread>
#include <list>

#include "../../../Source/Jinx.h"

#ifdef JINX_WINDOWS
#include <conio.h>
#endif

using namespace Jinx;


static const char * s_testScripts[] =
{
	u8R"(

		loop from 1 to 10
			set a to 2
			set b to 3
			set c to a + b
			set d to b - a
			set e to a * b
			set f to b / 1
			set g to 10 % b
			set h to 123.456
			set i to 23.45
			set j to h * i
			set k to h / i
			set l to h + i
			set m to h - i
			set n to h % i
			wait
		end

	)",
	
	u8R"(

		loop from 1 to 10
			set a to true and false
			set b to 1 = 2 or 3 < 4
			set c to 1 = 2 or 3 < 4 and 5 = 6
			set d to 1 != 2 or 3 >= 3 and 5 = 5
			set e to ((1 + 2) * 3) = ((1 + 2) * 3) and (1 + 2) * 3 < (1 + 2) * 4
			set f to 1 + 5 < 1 + 6
			wait
		end

	)",
	
	u8R"(
	
		-- Simple if/else tests

		set a to false
		if true
			set a to true
		end

		set b to false
		if true
			set b to true
		else
			set b to false
		end

		set c to false
		if false
			set c to false
		else
			set c to true
		end

		set d to false
		if false
			set d to false
		else if true
			set d to true
		else
			set d to false
		end

		set e to false
		if false
			set e to false
		else if false
			set e to false
		else if true
			set e to true
		else
			set e to false
		end

		set f to false
		if false
			set f to false
		else if false
			set f to false
		else if false
			set f to false
		else
			set f to true
		end

		set g to false
		if true
			if true
				set g to true
			end
		end

		set h to false
		if false
		else
			if true
				set h to true
			end
		end

		)",
		
		u8R"(
		import core

		loop from 1 to 100
			set a to [1, "red"], [2, "green"], [3, "blue"], [4, "yellow"], [5, "magenta"], [6, "cyan"]
			set b to a[1]
			set c to a[2]
			set d to a[3]
			set e to a[4]
			set a[5] to "purple"
			set a[6] to "black"
			loop i over a
				if i value = "blue"
					erase i
				end
			end
			wait
		end

		)",
		
};
		
const size_t NumPermutations = 5000;

template<typename T, size_t s>
constexpr size_t countof(T(&)[s])
{
	return s;
}

int main(int argc, char * argv[])
{
	Jinx::GlobalParams globalParams;
	Jinx::Initialize(globalParams);

	std::array<Jinx::BufferPtr, countof(s_testScripts)> bytecodeArray;

	// Create runtime for benchmark testing
	auto runtime = CreateRuntime();

	// Compile all test scripts
	for (size_t i = 0; i < countof(s_testScripts); ++i)
	{
		// Compile the text to bytecode
		auto bytecode = runtime->Compile(s_testScripts[i], "Test Script", { "core" });
		assert(bytecode);
		bytecodeArray[i] = bytecode;

		// Create a runtime script with the given bytecode
		auto script = runtime->CreateScript(bytecode);
		assert(bytecode);
		assert(script->Execute());
	}

	std::list<std::thread> threadList;
	auto numCores = std::thread::hardware_concurrency();
	//numCores = 1;
	for (unsigned int n = 0; n < numCores; ++n)
	{
		threadList.push_back(std::thread([bytecodeArray, runtime, numCores]()
		{
			// Run performance tests on bytecode
			auto numTests = NumPermutations / numCores;
			for (auto j = 0; j < numTests; ++j)
			{
				for (int i = 0; i < bytecodeArray.size(); ++i)
				{
					// Create a runtime script with the given bytecode if it exists
					auto script = runtime->CreateScript(bytecodeArray[i]);
					assert(script);
					do
					{
						script->Execute();
					}
					while (!script->IsFinished());
				}
			}
		}));
	}

	// Wait for all threads to finish
	for (auto & t : threadList)
		t.join();

	auto perfStats = runtime->GetScriptPerformanceStats();
	double executionTime = (double)perfStats.executionTimeNs / 1000000000;
	double perfRunTime = (double)perfStats.perfTimeNs / 1000000000;
	printf("--- Performance ---\n");
	printf("Total execution time: %f seconds\n", executionTime);
	printf("Total run time: %f seconds\n", perfRunTime);
	printf("Number of scripts executed: %llu (%llu per second)\n", perfStats.scriptExecutionCount, (uint64_t)((double)perfStats.scriptExecutionCount / perfRunTime));
	printf("Number of scripts completed: %llu (%llu per second)\n", perfStats.scriptCompletionCount, (uint64_t)((double)perfStats.scriptCompletionCount / perfRunTime));
	printf("Number of instructions executed: %llu (%.2fM per second)\n", perfStats.instructionCount, ((double)perfStats.instructionCount / perfRunTime / 1000000.0));

	bytecodeArray.fill(nullptr);
	runtime = nullptr;

	auto memoryStats = Jinx::GetMemoryStats();
	printf("\n--- Memory ---\n");
	printf("External alloc count: %llu\n", memoryStats.externalAllocCount);
	printf("External free count: %llu\n", memoryStats.externalFreeCount);
	printf("Internal alloc count: %llu\n", memoryStats.internalAllocCount);
	printf("Internal free count: %llu\n", memoryStats.internalFreeCount);
	printf("Current block count: %llu\n", memoryStats.currentBlockCount);
	printf("Current allocated memory: %llu bytes\n", memoryStats.currentAllocatedMemory);
	printf("Current used memory: %llu bytes\n", memoryStats.currentUsedMemory);
	printf("\n");

#ifdef JINX_WINDOWS
	printf("Press any key to continue...");
	_getch();
#endif

	return 0;
}