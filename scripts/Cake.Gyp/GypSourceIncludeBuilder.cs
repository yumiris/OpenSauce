﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Cake.Core.IO;
using Newtonsoft.Json;

namespace Cake.Gyp
{
	// NOTE: There is no automatic functionality in gyp to trawl a directory for source files.
	// The include builder handles that functionality.

	public sealed class GypSourceIncludeBuilder
	{
		private readonly IFileSystem mFileSystem;
		private readonly DirectoryPath mRootDirectory;

		public GypSourceIncludeBuilder(IFileSystem fileSystem, DirectoryPath rootDirectory)
		{
			if (fileSystem == null)
			{
				throw new ArgumentNullException(nameof(fileSystem));
			}
			if (rootDirectory == null)
			{
				throw new ArgumentNullException(nameof(rootDirectory));
			}
			mFileSystem = fileSystem;
			mRootDirectory = rootDirectory;
		}

		public void GenerateSourceInclude(DirectoryPath sourceDirectory, DirectoryPath relativeToDirectory, IEnumerable<string> filePatterns, FilePath outputFile, string outputVariable)
		{
			var paths = GetSourceFiles(sourceDirectory, relativeToDirectory, filePatterns);
			WriteGypInclude(outputFile, CreateIncludeObject(outputVariable, paths.ToArray()));
		}

		private IEnumerable<string> GetSourceFiles(DirectoryPath sourceDirectory, DirectoryPath relativeToDirectory, IEnumerable<string> filePatterns)
		{
			var searchPath = mRootDirectory.Combine(sourceDirectory);
			var relativePath = mRootDirectory.Combine(relativeToDirectory);

			var sourceFiles = GetFilesMatchingPatterns(filePatterns, searchPath);
			return GetRelativeFilePaths(sourceFiles, searchPath, relativePath);
		}

		private IEnumerable<FilePath> GetFilesMatchingPatterns(IEnumerable<string> filePatterns, DirectoryPath searchPath)
		{
			return filePatterns.SelectMany(entry => GetFilesMatchingPattern(searchPath, entry));
		}

		private IEnumerable<FilePath> GetFilesMatchingPattern(DirectoryPath searchPath, string pattern)
		{
			return mFileSystem.GetDirectory(searchPath)
			                  .GetFiles(pattern, SearchScope.Recursive)
			                  .Select(entry => entry.Path);
		}

		private static IEnumerable<string> GetRelativeFilePaths(IEnumerable<FilePath> sourceFiles, DirectoryPath sourcePath, DirectoryPath targetPath)
		{
			var targetRelativePath = targetPath.GetRelativePath(sourcePath);
			return sourceFiles.Select(entry => targetRelativePath.CombineWithFilePath(entry.ToString()
			                                                                               .Replace(sourcePath.FullPath, string.Empty)
			                                                                               .TrimStart('/')))
			                  .Select(entry => entry.ToString());
		}

		private void WriteGypInclude(FilePath outputFile, object includeData)
		{
			using (var streamWriter = new StreamWriter(mFileSystem.GetFile(outputFile).OpenWrite()))
			{
				streamWriter.Write(JsonConvert.SerializeObject(includeData, Formatting.Indented));
			}
		}

		private static object CreateIncludeObject(string outputVariable, string[] paths)
		{
			return new
			{
				variables = new Dictionary<string, object>
				{
					[outputVariable] = paths
				}
			};
		}
	}
}