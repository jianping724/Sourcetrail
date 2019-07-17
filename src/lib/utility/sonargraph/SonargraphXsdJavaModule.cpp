#include "SonargraphXsdJavaModule.h"

#include "tinyxml.h"

#include "FileSystem.h"
#include "IndexerCommandJava.h"
#include "LanguageType.h"
#include "logging.h"
#include "SonargraphSourceRootPath.h"
#include "SonargraphSoftwareSystem.h"
#include "SonargraphXsdRootPath.h"
#include "SourceGroupSettingsJavaSonargraph.h"
#include "utility.h"
#include "utilityFile.h"
#include "utilitySonargraph.h"
#include "utilityXml.h"

namespace Sonargraph
{
	std::string XsdJavaModule::getXsdTypeName()
	{
		return "xsdJavaModule";
	}

	std::shared_ptr<XsdJavaModule> XsdJavaModule::create(const TiXmlElement* element, std::weak_ptr<SoftwareSystem> parent)
	{
		if (!utility::xmlElementHasAttribute(element, "xsi:type") || utility::sonargraphXmlElementIsType(element, getXsdTypeName()))
		{
			std::shared_ptr<XsdJavaModule> module = std::shared_ptr<XsdJavaModule>(new XsdJavaModule());
			if (module->init(element, parent))
			{
				return module;
			}
		}
		return std::shared_ptr<XsdJavaModule>();
	}

	LanguageType XsdJavaModule::getSupportedLanguage() const
	{
		return LANGUAGE_JAVA;
	}

	std::set<FilePath> XsdJavaModule::getAllSourcePaths() const
	{
		const FilePath baseDirectory = getSoftwareSystem() ? getSoftwareSystem()->getBaseDirectory() : FilePath();

		std::set<FilePath> sourcePaths;
		for (std::shared_ptr<XsdRootPath> rootPath : getRootPaths())
		{
			sourcePaths.insert(rootPath->getFilePath(baseDirectory));
		}

		for (std::shared_ptr<SourceRootPath> rootPath : m_sourceRootPaths)
		{
			sourcePaths.insert(rootPath->getFilePath(baseDirectory));
		}

		return sourcePaths;
	}

	std::set<FilePath> XsdJavaModule::getAllSourceFilePathsCanonical() const
	{
		const std::vector<FilePathFilter> excludeFilters = getDerivedExcludeFilters();
		const std::vector<FilePathFilter> includeFilters = getDerivedIncludeFilters();

		std::set<FilePath> sourceFilePaths;

		for (const FileInfo& fileInfo : FileSystem::getFileInfosFromPaths(utility::getTopLevelPaths(getAllSourcePaths()), { L".java" }))
		{
			const FilePath sourceFilePath = fileInfo.path.getCanonical();

			bool excludeMatches = false;
			for (const FilePathFilter& excludeFilter : excludeFilters)
			{
				if (excludeFilter.isMatching(sourceFilePath))
				{
					excludeMatches = true;
					break;
				}
			}
			if (excludeMatches)
			{
				for (const FilePathFilter& includeFilter : includeFilters)
				{
					if (includeFilter.isMatching(sourceFilePath))
					{
						excludeMatches = false;
						break;
					}
				}
			}
			if (!excludeMatches)
			{
				sourceFilePaths.insert(sourceFilePath);
			}
		}

		return sourceFilePaths;
	}

	std::set<FilePath> XsdJavaModule::getAllCxxHeaderSearchPathsCanonical() const
	{
		return std::set<FilePath>();
	}

	std::vector<std::shared_ptr<IndexerCommand>> XsdJavaModule::getIndexerCommands(
		std::shared_ptr<const SourceGroupSettings> sourceGroupSettings,
		std::shared_ptr<const ApplicationSettings> appSettings) const
	{
		std::vector<std::shared_ptr<IndexerCommand>> indexerCommands;

		{
			std::wstring languageStandard = SourceGroupSettingsWithJavaStandard::getDefaultJavaStandardStatic();
			if (std::shared_ptr<const SourceGroupSettingsJavaSonargraph> settings =
				std::dynamic_pointer_cast<const SourceGroupSettingsJavaSonargraph>(sourceGroupSettings))
			{
				languageStandard = settings->getJavaStandard();
			}
			else
			{
				LOG_ERROR(L"Source group doesn't specify language standard. Falling back to \"" + languageStandard + L"\".");
			}

			for (const FilePath& sourceFilePath : getAllSourceFilePathsCanonical())
			{
				indexerCommands.push_back(std::make_shared<IndexerCommandJava>(
					sourceFilePath,
					languageStandard,
					std::vector<FilePath>() // the classpath is set later... TODO: fix this hack
				));
			}
		}
		return indexerCommands;
	}

	bool XsdJavaModule::init(const TiXmlElement* element, std::weak_ptr<SoftwareSystem> parent)
	{
		if (!XsdAbstractModule::init(element, parent))
		{
			return false;
		}

		if (element != nullptr)
		{
			for (const TiXmlElement* sourceRootPathElement : utility::getXmlChildElementsWithName(element, "sourceRootPath"))
			{
				if (std::shared_ptr<SourceRootPath> rootPath = SourceRootPath::create(sourceRootPathElement))
				{
					m_sourceRootPaths.push_back(rootPath);
				}
				else
				{
					LOG_ERROR("Unable to parse \"sourceRootPath\" element of Sonargraph " + getXsdTypeName() + ".");
					return false;
				}
			}
			return true;
		}
		return false;
	}
}
