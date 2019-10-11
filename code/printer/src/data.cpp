//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include <common/algorithms/rwls.hpp>
#include "printer/data.hpp"
#include "common/algorithms/greedy.hpp"
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

bool uscp::data::load(const nlohmann::json& data, printer& printer) noexcept
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

		it = data.find("date");
		if(it != data.end())
		{
			LOGGER->info("data generation date: {}", it->get<std::string>());
		}
		else
		{
			LOGGER->warn("data is missing date information");
		}

		it = data.find("instances");
		if(it == data.end())
		{
			LOGGER->warn("data is missing instances information");
			return false;
		}

		const nlohmann::json& instances_data = *it;
		if(!instances_data.is_array())
		{
			LOGGER->warn("data have invalid instances information");
			return false;
		}
		for(const nlohmann::json& instance_data: instances_data)
		{
			it = instance_data.find("instance");
			if(it == instance_data.end())
			{
				LOGGER->warn("instance data is missing instance information");
				continue;
			}
			const uscp::problem::instance_serial instance =
			  it->get<uscp::problem::instance_serial>();
			SPDLOG_LOGGER_DEBUG(LOGGER, "Started loading data for instance {}", instance.name);

			it = instance_data.find("greedy");
			if(it != instance_data.end())
			{
				const uscp::greedy::report_serial greedy = it->get<uscp::greedy::report_serial>();
				SPDLOG_LOGGER_DEBUG(LOGGER, "Loaded greedy solution with {} subsets", greedy.solution_final.selected_subsets.size());
				printer.add(greedy);
			}

			it = instance_data.find("rwls");
			if(it != instance_data.end())
			{
				const nlohmann::json& rwls_reports_data = *it;
				if(!rwls_reports_data.is_array())
				{
					LOGGER->warn("data have invalid rwls information for instance {}", instance.name);
					continue;
				}
				for(const nlohmann::json& rwls_report_data: rwls_reports_data)
				{
					const uscp::rwls::report_serial rwls = rwls_report_data.get<uscp::rwls::report_serial>();
					SPDLOG_LOGGER_DEBUG(LOGGER, "Loaded rwls solution with {} subsets", rwls.solution_final.selected_subsets.size());
					printer.add(rwls);
				}
			}
		}
	}
	catch(const std::exception& e)
	{
		LOGGER->error("error loading data: {}", e.what());
		return false;
	}
	catch(...)
	{
		LOGGER->error("unknown error loading data");
		return false;
	}
	return true;
}
