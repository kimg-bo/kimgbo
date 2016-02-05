#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include<stdint.h>
#include<time.h>
#include<iostream>
#include<string>

using namespace std;

namespace kimgbo
{
	class Timestamp
	{
	public:
		Timestamp():m_microSecondsSinceEpoch(0)
		{
		}
		
		explicit Timestamp(int64_t microSecondsSinceEpoch);
		
		void swap(Timestamp& that)
		{
			std::swap(m_microSecondsSinceEpoch, that.m_microSecondsSinceEpoch);
		}

		std::string toString() const;
		
		std::string toFormattedString() const;
		
		bool valid() const { return m_microSecondsSinceEpoch>0; }
		
		  // for internal usage.
  	int64_t microSecondsSinceEpoch() const 
  	{ 
  		return m_microSecondsSinceEpoch; 
  	}
  	
  	time_t secondsSinceEpoch() const
  	{ 
  		return static_cast<time_t>(m_microSecondsSinceEpoch / kMicroSecondsPerSecond); 
  	}
		
		static Timestamp now();
		static Timestamp invalid();
	
		static const int kMicroSecondsPerSecond = 1000*1000;
		
	public:
		inline friend bool operator<(Timestamp lhs, Timestamp rhs)
		{
			return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
		}

		inline friend bool operator>(Timestamp lhs, Timestamp rhs)
		{
			return lhs.microSecondsSinceEpoch() > rhs.microSecondsSinceEpoch();
		}

		inline friend bool operator==(Timestamp lhs, Timestamp rhs)
		{
  		return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
		}
		
	private:
		int64_t m_microSecondsSinceEpoch;
	};
			
	inline double timeDifference(Timestamp high, Timestamp low)
	{
		int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
		return static_cast<double>(diff)/Timestamp::kMicroSecondsPerSecond;
	}

	inline Timestamp addTimer(Timestamp timestamp, double seconds)
	{
		int64_t delta = static_cast<int64_t>(seconds*Timestamp::kMicroSecondsPerSecond);
		return Timestamp((timestamp.microSecondsSinceEpoch() + delta));
	}
}

#endif