// Code based on https://github.com/jbikker/lighthouse2/blob/master/lib/rendercore_vulkan_rt/vulkan_shader.h
#pragma once

#include "core/pch.hpp"
#include "graphics/context/context.hpp"

#include <shaderc/shaderc.hpp>

namespace PXTEngine
{

	class VulkanShader
	{
	public:
		VulkanShader() = default;
		VulkanShader(Context& context, const std::string_view& fileName, const std::vector<std::pair<std::string, std::string>>& definitions = {});
		~VulkanShader();

		void cleanup();

		VkPipelineShaderStageCreateInfo getShaderStageCreateInfo();

		VkShaderModule getShaderModule() { return m_module; }

		std::string preprocessShader(const std::string_view& fileName, const std::string& source,
			shaderc_shader_kind shaderKind = shaderc_glsl_infer_from_source);

		std::string compileToAssembly(const std::string_view& fileName, const std::string& source,
			shaderc_shader_kind shaderKind = shaderc_glsl_infer_from_source);

		std::vector<uint32_t> compileFile(const std::string_view& fileName, const std::string& source,
			shaderc_shader_kind shaderKind = shaderc_glsl_infer_from_source);

	private:
		// Helper classes taken from glslc: https://github.com/google/shaderc
		class FileFinder
		{
		public:
			std::string findReadableFilepath(const std::string& filename) const;

			std::string findRelativeReadableFilepath(const std::string& requesting_file, const std::string& filename) const;

			std::vector<std::string>& getSearchPath() { return m_searchPath; }

		private:
			std::vector<std::string> m_searchPath;
		};
		class FileIncluder : public shaderc::CompileOptions::IncluderInterface
		{
		public:
			explicit FileIncluder(const FileFinder* file_finder) : m_fileFinder(*file_finder) {}

			~FileIncluder() override;

			shaderc_include_result* GetInclude(const char* requested_source,
				shaderc_include_type type,
				const char* requesting_source,
				size_t include_depth) override;

			void ReleaseInclude(shaderc_include_result* include_result) override;

			const std::unordered_set<std::string>& getIncludedFiles() const { return m_includedFiles; }

		private:
			const FileFinder& m_fileFinder;

			struct FileInfo
			{
				const std::string fullPath;
				std::vector<char> contents;
			};
			std::unordered_set<std::string> m_includedFiles;
		};

		void inferKindAndStageFromFileName(const std::string_view& fileName);

		Context& m_context;
		shaderc::Compiler m_compiler;
		shaderc::CompileOptions m_compileOptions;
		FileFinder m_finder{};
		VkShaderModule m_module = nullptr;

		shaderc_shader_kind m_kind = shaderc_glsl_infer_from_source;
		VkShaderStageFlagBits m_vkStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM; // Default to an invalid stage

		static std::vector<char> readFile(const std::string_view& fileName);
		static std::string readTextFile(const std::string_view& fileName);
	};
}