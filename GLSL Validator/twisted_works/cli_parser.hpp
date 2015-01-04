#ifndef HG_TWISTED_WORKS_CLI_PARSER_HPP
#define HG_TWISTED_WORKS_CLI_PARSER_HPP

#pragma once

/*
Copyright (c) , 2008, Twisted Works / Hisham Younis
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright 
notice, this list of conditions and the following disclaimer. 

* Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in 
the documentation and/or other materials provided with the distribution. 

* Neither the name of Twisted Works nor the names of its contributors 
may be used to endorse or promote products derived from this software 
without specific prior written permission. 

* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <vector>
#include <map>
#include <string>

namespace twisted_works
{
    template<class CharType, class StringType>
    class cli_parser_traits
    {
    };

    template<>
    class cli_parser_traits<char, std::string>
    {
    public:
        static std::string specifier() { return "+"; };
        static std::string assignment() { return "="; };
        static std::string quote_begin() { return "\""; };
        static std::string quote_end() { return "\""; };
        static std::string escape() { return "\\"; };
    };

    template<class CharType, class StringType, class Traits = cli_parser_traits<CharType, StringType>, template<class AllocatedType> class Allocator = std::allocator>
	class basic_cli_parser
	{
	public:
		//Typedefs
		typedef unsigned int size_type;
		typedef StringType string_type;
		basic_cli_parser(int argc, CharType** argv);
		size_type size() const;
		bool empty() const;
		bool is_named_argument(const string_type& ArgumentName) const;
        size_type anonymous_argument_count() const;
		void anonymous_argument(const size_type AnonymousArgumentNumber, const string_type& NewValue);
		string_type anonymous_argument(const size_type AnonymousArgumentNumber) const;
		void argument(const string_type& ArgumentName, const string_type& NewValue);
		string_type argument(const string_type& ArgumentName) const;
	private:
		static string_type unescape(string_type Str);
		static typename string_type::size_type find_excluding_quotes(string_type Str, const string_type& Rhs);
		static typename string_type::size_type find(const string_type& Haystack, const string_type& Needle, const typename string_type::size_type Offset = 0);
		static string_type merge(const string_type& First, const string_type& Second);
		static string_type substr(const string_type& Str, const typename string_type::size_type Offset, const typename string_type::size_type Count);
	private:
		std::vector<std::pair<string_type, string_type>, Allocator<std::pair<string_type, string_type> > > arguments;
	};

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	basic_cli_parser<CharType, StringType, Traits, Allocator>::basic_cli_parser(int argc, CharType** argv)
	{
		for(int i = 1; i < argc; ++i)
		{
			string_type Arg = argv[i];
			std::pair<string_type, string_type> ArgPair;
			
			//If there isn't a specifier at the beginning of the current argument.
			if(find(Arg, Traits::specifier()) != 0)
			{
				//Remove any possible escape characters in the string.
				Arg = unescape(Arg);

				//Add the argument as an anonymous argument.
				ArgPair.second = Arg;
				arguments.push_back(ArgPair);

				//Get the next argument.
				continue;
			}
			//If there is a specifier at the beginning of the current argument.
			//Remove the specifier from the string.
			Arg = substr(Arg, Traits::specifier().size(), Arg.size());

			//If there isn't an assignment string in the current argument.
			if(find_excluding_quotes(Arg, Traits::assignment()) == typename string_type::size_type(-1))
			{
				//Remove any possible escape characters in the string.
				Arg = unescape(Arg);

				//Add the argument as a value-less argument.
				ArgPair.first = Arg;
				arguments.push_back(ArgPair);

				//Get the next argument.
				continue;
			}
			//If there is an assignment string in the current argument.
			//Split the argument into two parts.

			//Argument name.
			ArgPair.first = substr(Arg, 0, find_excluding_quotes(Arg, Traits::assignment()));

			//Remove any possible escape characters in the string.
			ArgPair.first = unescape(ArgPair.first);

			//Argument value.
			ArgPair.second = substr(Arg, find_excluding_quotes(Arg, Traits::assignment()) + 1, Arg.size());

			//Remove any possible escape characters in the string.
			ArgPair.second = unescape(ArgPair.second);

			//Add the argument to the arguments vector.
			arguments.push_back(ArgPair);

			//Get the next argument.
		}
	}

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	typename basic_cli_parser<CharType, StringType, Traits, Allocator>::string_type basic_cli_parser<CharType, StringType, Traits, Allocator>::unescape(string_type Str)
	{
		typename string_type::size_type pos = find(Str, Traits::escape(), 0);

		//While there is an escape character in the string.
		while(pos != typename string_type::size_type(-1))
		{
			//Split the string into two parts, excluding the escape character.
			string_type First, Second;

			//The substring before the escape character.
			First = substr(Str, 0, find(Str, Traits::escape()));

			//The substring after the escape character.
			Second = substr(Str, find(Str, Traits::escape()) + Traits::escape().size(), Str.size());

			//Merge the two substrings together.
			Str = merge(First, Second);

			pos = find(Str, Traits::escape(), pos);
		}
		return Str;
	}

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	typename basic_cli_parser<CharType, StringType, Traits, Allocator>::string_type::size_type basic_cli_parser<CharType, StringType, Traits, Allocator>::find_excluding_quotes(string_type Str, const string_type& Rhs)
	{
		std::vector<std::pair<typename string_type::size_type, typename string_type::size_type>, Allocator<std::pair<typename string_type::size_type, typename string_type::size_type> > > QuotedIntervals;
		typename string_type::size_type pos = find(Str, Traits::quote_begin());
		
		//While there is a quote begin in the string.
		while(pos != typename string_type::size_type(-1))
		{
			//If there is an escape character before the quote begin.
			if(pos >= Traits::escape().size())
			{
				if(find(Str, Traits::escape(), pos - Traits::escape().size()) == pos - Traits::escape().size())
				{
					//Continue searching from the current position onward.
					pos = find(Str, Traits::quote_begin(), pos + 1);
					continue;
				}
			}
			//If there is no escape character before the quote begin.
			//Save the position of the quote begin.
			typename string_type::size_type BeginPos = pos;

			typename string_type::size_type EndPos = Str.size();

			//Find the quote end that corresponds to the current quote begin.
			pos = find(Str, Traits::quote_end(), pos + 1);
			while(pos != typename string_type::size_type(1))
			{
				//If there is an escape character before the quote end.
				if(pos >= Traits::escape().size())
				{
					if(find(Str, Traits::escape(), pos - Traits::escape().size()) == pos - Traits::escape().size())
					{
						//Continue searching from the current position onward.
						pos = find(Str, Traits::quote_end(), pos + 1);
						continue;
					}
				}
				//If there is no escape character before the quote end.
				//Save the position of the quote end.
				EndPos = pos;
				break;
			}
			//Split the string into two parts, excluding the quoted string.
			string_type First, Second;

			//The substring before the quoted string.
			First = substr(Str, 0, BeginPos);

			//The substring after the quoted string.
			Second = substr(Str, EndPos + 1, Str.size());

			//Merge the two substrings.
			Str = merge(First, Second);

			//Insert the positions of the quote begin and quote end to mark the deleted interval.
			QuotedIntervals.push_back(std::pair<typename string_type::size_type, typename string_type::size_type>(BeginPos, EndPos));

			pos = find(Str, Traits::quote_begin(), 0);
			//Continue searching for quoted strings.
		}
		pos = find(Str, Rhs);

		typename string_type::size_type Ret = pos;

		//If the string being searched for is found
		if(pos != typename string_type::size_type(-1))
		{
			//Iterate over the deleted intervals adding the count of deleted characters to the offset.
			for(unsigned int i = 0; i < QuotedIntervals.size(); ++i)
			{
				if(QuotedIntervals[i].first <= pos)
				{
					Ret += QuotedIntervals[i].second - QuotedIntervals[i].first + 1;
				}
			}
			return Ret;
		}
		return -1;
	}

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	typename basic_cli_parser<CharType, StringType, Traits, Allocator>::string_type basic_cli_parser<CharType, StringType, Traits, Allocator>::merge(const string_type& First, const string_type& Second)
	{
		std::vector<CharType, Allocator<CharType> > V;
		for(unsigned int i = 0; i < First.size(); ++i)
		{
			V.push_back(First[i]);
		}
		for(unsigned int i = 0; i < Second.size(); ++i)
		{
			V.push_back(Second[i]);
		}
		string_type Str(&V[0], V.size());
		return Str;
	}

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	typename basic_cli_parser<CharType, StringType, Traits, Allocator>::string_type::size_type basic_cli_parser<CharType, StringType, Traits, Allocator>::find(const string_type& Haystack, const string_type& Needle, const typename string_type::size_type Offset)
	{
		if(Needle.empty())
		{
			return -1;
		}
		for(typename string_type::size_type i = Offset; i < Haystack.size() - Needle.size() + 1; ++i)
		{
			bool flag = true;
			for(typename string_type::size_type j = 0; j < Needle.size(); ++j)
			{
				if(Haystack[i + j] != Needle[j])
				{
					flag = false;
					break;
				}
			}
			if(flag)
				return i;
		}
		return -1;
	}

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	typename basic_cli_parser<CharType, StringType, Traits, Allocator>::string_type basic_cli_parser<CharType, StringType, Traits, Allocator>::substr(const string_type& Str, const typename string_type::size_type Offset, const typename string_type::size_type Count)
	{
		std::vector<CharType, Allocator<CharType> > V;
		for(typename string_type::size_type i = Offset; i < Offset + Count; ++i)
		{
			if(i == Str.size())
			{
				break;
			}
			V.push_back(Str[i]);
		}
		if(V.empty())
		{
			return string_type();
		}
		string_type Ret(&V[0], V.size());
		return Ret;
	}

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	typename basic_cli_parser<CharType, StringType, Traits, Allocator>::size_type basic_cli_parser<CharType, StringType, Traits, Allocator>::size() const
	{
		return basic_cli_parser::size_type(arguments.size());
	}

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	bool basic_cli_parser<CharType, StringType, Traits, Allocator>::empty() const
	{
		return arguments.empty();
	}

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	bool basic_cli_parser<CharType, StringType, Traits, Allocator>::is_named_argument(const string_type& ArgumentName) const
	{
		for(unsigned int i = 0; i < arguments.size(); ++i)
		{
			if(arguments[i].first == ArgumentName)
			{
				return true;
			}
		}
		return false;
	}

    template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
    typename basic_cli_parser<CharType, StringType, Traits, Allocator>::size_type basic_cli_parser<CharType, StringType, Traits, Allocator>::anonymous_argument_count() const
    {
        typename basic_cli_parser::size_type Count = 0;
        for(typename basic_cli_parser::size_type i = 0; i < arguments.size(); ++i)
        {
            if(arguments[i].first.empty())
            {
                ++Count;
            }
        }
        return Count;
    }

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	void basic_cli_parser<CharType, StringType, Traits, Allocator>::anonymous_argument(const size_type AnonymousArgumentNumber, const string_type& NewValue)
	{
		typename basic_cli_parser::size_type Index = 0;
		for(typename basic_cli_parser::size_type i = 0; i < arguments.size(); ++i)
		{
			if(arguments[i].first == "")
			{
				++Index;
			}
			if(Index == AnonymousArgumentNumber + 1)
			{
				arguments[i].second = NewValue;
				return;
			}
		}
	}

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	typename basic_cli_parser<CharType, StringType, Traits, Allocator>::string_type basic_cli_parser<CharType, StringType, Traits, Allocator>::anonymous_argument(const size_type AnonymousArgumentNumber) const
	{
		typename basic_cli_parser::size_type Index = 0;
		for(typename basic_cli_parser::size_type i = 0; i < arguments.size(); ++i)
		{
			if(arguments[i].first == "")
			{
				++Index;
			}
			if(Index == AnonymousArgumentNumber + 1)
			{
				return arguments[i].second;
			}
		}
		return basic_cli_parser::string_type();
	}

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	void basic_cli_parser<CharType, StringType, Traits, Allocator>::argument(const string_type& ArgumentName, const string_type& NewValue)
	{
		for(unsigned int i = 0; i < arguments.size(); ++i)
		{
			if(arguments[i].first == ArgumentName)
			{
				arguments[i].second = NewValue;
			}
		}	
	}

	template<class CharType, class StringType, class Traits, template<class alloc> class Allocator>
	typename basic_cli_parser<CharType, StringType, Traits, Allocator>::string_type basic_cli_parser<CharType, StringType, Traits, Allocator>::argument(const string_type& ArgumentName) const
	{
		for(unsigned int i = 0; i < arguments.size(); ++i)
		{
			if(arguments[i].first == ArgumentName)
			{
				return arguments[i].second;
			}
		}
		return string_type();
	}
    //
    typedef basic_cli_parser<char, std::string, cli_parser_traits<char, std::string>, std::allocator> cli_parser;
    typedef basic_cli_parser<wchar_t, std::wstring, cli_parser_traits<wchar_t, std::wstring>, std::allocator> wcli_parser;
}

#endif
