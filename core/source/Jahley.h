//
// Note:	this file is to be included in client applications ONLY
//			NEVER include this file anywhere in the engine codebase
//
#pragma once

#include "jahley/AppConfig.h"
#include "jahley/core/App.h"
#include "jahley/EntryPoint.h"

#include <filesystem>
#include <string>

// Repository name for searching in the path
const std::string REPOSITORY_NAME = "NanoBrain";

// Function to get the path for common content folder
inline std::string getCommonContentFolder()
{
    // Get the current working directory
    std::filesystem::path exePath = std::filesystem::current_path();
    std::string fullPath = exePath.string();

    // Find the position of the repository name in the path
    std::string::size_type pos = fullPath.find (REPOSITORY_NAME);

    // Extract the path up to the repository
    std::string pathToRepro = fullPath.substr (0, pos + std::string (REPOSITORY_NAME).length());

    // Create the common folder path string
    std::string commonFolder = pathToRepro + "/resources/Common/";

    // Create the folder if it doesn't exist
    std::filesystem::create_directories (commonFolder);

    return commonFolder;
}

// Function to get the resource path for a given application name
inline std::string getResourcePath (const std::string& appName)
{
    // Get the current working directory
    std::filesystem::path exePath = std::filesystem::current_path();
    std::string fullPath = exePath.string();

    // Find the position of the repository name in the path
    std::string::size_type pos = fullPath.find (REPOSITORY_NAME);

    // Extract the path up to the repository
    std::string pathToRepro = fullPath.substr (0, pos + std::string (REPOSITORY_NAME).length());

    // Create the resource folder path string
    std::string resourceFolder = pathToRepro + "/resources/" + appName;

    // Create the folder if it doesn't exist
    std::filesystem::create_directories (resourceFolder);

    // Return the resource folder path
    return resourceFolder;
}

// Function to get the path to the repository
inline std::string getRepositoryPath (const std::string& appName)
{
    // Get the current working directory
    std::filesystem::path exePath = std::filesystem::current_path();
    std::string fullPath = exePath.string();

    // Find the position of the repository name in the path
    std::string::size_type pos = fullPath.find (REPOSITORY_NAME);

    // Extract the path up to the repository
    std::string pathToRepro = fullPath.substr (0, pos + std::string (REPOSITORY_NAME).length());

    // Return the repository path
    return pathToRepro;
}



 