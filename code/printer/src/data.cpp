//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "printer/data.hpp"
#include "common/utils/logger.hpp"
#include "git_info.hpp"

namespace
{
	void check_git(const nlohmann::json& data) noexcept;

	void check_git(const nlohmann::json& data) noexcept
	{
		try
		{
			if(!git_info::retrieved_state)
			{
				LOGGER->warn("program have invalid git information: git information check skipped");
				return;
			}
			if(!git_info::retrieved_state)
			{
				LOGGER->warn("program was compiled with uncommited modifications");
			}

			nlohmann::json::const_iterator it = data.find("retrieved_state");
			if(it != data.end())
			{
				if(!it->get<bool>())
				{
					LOGGER->warn("data without valid git information");
					return;
				}
			}
			else
			{
				LOGGER->warn("data is missing git retrieved_state information");
			}

			it = data.find("is_dirty");
			if(it != data.end())
			{
				if(it->get<bool>())
				{
					LOGGER->warn("data was generated with uncommited modifications on the project");
				}
			}
			else
			{
				LOGGER->warn("data is missing git is_dirty information");
			}

			it = data.find("head_sha1");
			if(it != data.end())
			{
				if(it->get<std::string>() != git_info::head_sha1)
				{
					LOGGER->warn(
					  "data was generated with a different version of the project (program: {}, data: {})",
					  git_info::head_sha1,
					  it->get<std::string>());
				}
			}
			else
			{
				LOGGER->warn("data is missing git head_sha1 information");
			}
		}
		catch(const std::exception& e)
		{
			LOGGER->error("error processing git data: {}", e.what());
		}
		catch(...)
		{
			LOGGER->error("unknown error processing data");
		}
	}
} // namespace

bool uscp::data::process(const nlohmann::json& data, printer& printer) noexcept
{
	try
	{
		nlohmann::json::const_iterator it = data.find("git");
		if(it != data.end())
		{
			check_git(*it);
		}
		else
		{
			LOGGER->warn("data is missing git information");
		}

		//TODO
	}
	catch(const std::exception& e)
	{
		LOGGER->error("error processing data: {}", e.what());
		return false;
	}
	catch(...)
	{
		LOGGER->error("unknown error processing data");
		return false;
	}
	return true;
}
