#pragma once

#include <stdlib.h>
#include <vector>

namespace Maths
{
	namespace NoDistribution
	{
		// ---------------------------------

		static bool CoinFlip()
		{
			unsigned int randomNumber = unsigned int(rand() % 2);

			return randomNumber == 0 ? true : false;
		}

		// ---------------------------------

		static unsigned int DiceRoll_4()
		{
			unsigned int randomNumber = unsigned int(rand() % 4);

			return randomNumber + 1;
		}

		// ---------------------------------

		static unsigned int DiceRoll_6()
		{
			unsigned int randomNumber = unsigned int(rand() % 6);

			return randomNumber + 1;
		}

		// ---------------------------------

		static unsigned int DiceRoll_10()
		{
			unsigned int randomNumber = unsigned int(rand() % 10);

			return randomNumber + 1;
		}

		// ---------------------------------

		static unsigned int DiceRoll_20()
		{
			unsigned int randomNumber = unsigned int(rand() % 20);

			return randomNumber + 1;
		}

		// ---------------------------------

		static int RandomNumber(int start, unsigned int maxRange)
		{
			int randomNumber = (rand() % maxRange) + start;

			return randomNumber;
		}

		// ---------------------------------

		static float RandomNumber(float start, float maxRange, unsigned int decimalPlaces)
		{
			// Prevent divide by zero error
			if ((int)maxRange == 0)
				return start;

			// Decimal part
			unsigned int maxPrecision = (unsigned int)pow(10, decimalPlaces);
			float        decimal      = (rand() % maxPrecision) / float(maxPrecision);

			// Whole number part
			float        wholeNumberPart = float(rand() % (int)maxRange);
			wholeNumberPart             += start;

			// Now cap to the max range
			float finalNumber = wholeNumberPart + decimal;
			finalNumber       = std::min<float>(finalNumber, maxRange + start);

			return finalNumber;
		}

		// ---------------------------------

		struct RandomEvent
		{
			RandomEvent(float chance)
				: mPercentageChance(chance)
			{ }

			float mPercentageChance;
		};

		struct RandomCollection
		{
			std::vector<RandomEvent> mRandomEvents;
		};

		static int RandomOutcome(RandomCollection& collection)
		{
			// Find if the overall percentages were not 100%
			float        runningCount     = 0.0f;
			unsigned int randomEventCount = (unsigned int)collection.mRandomEvents.size();
			for (unsigned int i = 0; i < randomEventCount; i++)
			{
				runningCount += collection.mRandomEvents[i].mPercentageChance;
			}

			// Float error taken into account
			if (abs(runningCount - 100.0f) < 0.01f)
			{
				// Choose a random number
				float randomNumber = RandomNumber(0.0f, 100.0f, 3);

				// Now find where this number falls within the input
				runningCount = 0.0f;
				for (unsigned int i = 0; i < randomEventCount; i++)
				{
					runningCount += collection.mRandomEvents[i].mPercentageChance;

					if (float(randomNumber) < runningCount)
					{
						return (int)i;
					}
				}
			}

			return -1;
		}

		// ---------------------------------
	}
}