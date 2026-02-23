#include "ConfigMgr.h"
#include <vector>

bool ConfigMgr::LoadFromPath(const boost::filesystem::path& config_path) {
	if (!boost::filesystem::exists(config_path)) {
		return false;
	}

	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(config_path.string(), pt);
	_config_map.clear();

	for (const auto& section_pair : pt) {
		const std::string& section_name = section_pair.first;
		const boost::property_tree::ptree& section_tree = section_pair.second;

		std::map<std::string, std::string> section_config;
		for (const auto& key_value_pair : section_tree) {
			const std::string& key = key_value_pair.first;
			const std::string& value = key_value_pair.second.get_value<std::string>();
			section_config[key] = value;
		}

		SectionInfo sectionInfo;
		sectionInfo._section_datas = section_config;
		_config_map[section_name] = sectionInfo;
	}

	std::cout << "Loaded config from: " << config_path << std::endl;
	return true;
}

ConfigMgr::ConfigMgr() {
	const boost::filesystem::path current_path = boost::filesystem::current_path();
	const std::vector<boost::filesystem::path> candidate_paths = {
		current_path / "config.ini",
		current_path / "server" / "GateServer" / "config.ini"
	};

	for (const auto& candidate : candidate_paths) {
		try {
			if (LoadFromPath(candidate)) {
				return;
			}
		}
		catch (const std::exception& ex) {
			std::cout << "Failed to load config from " << candidate
				<< ", err: " << ex.what() << std::endl;
		}
	}

	std::cout << "Warning: config.ini not found. Running with defaults." << std::endl;
}
