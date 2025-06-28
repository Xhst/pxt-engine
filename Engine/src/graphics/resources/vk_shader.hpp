// Code based on https://github.com/jbikker/lighthouse2/blob/master/lib/rendercore_vulkan_rt/vulkan_shader.h

#pragma once

#include "graphics/context/context.hpp"
#include <unordered_set>

#include <shaderc/shaderc.hpp>

namespace PXTEngine
{

	class VulkanShader
	{
	public:
		VulkanShader() = default;
		VulkanShader(Context& context, const std::string_view& fileName, const std::vector<std::pair<std::string, std::string>>& definitions = {});
		~VulkanShader();

		void Cleanup();
		VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo();

		VkShaderModule getShaderModule() { return m_Module; }

		std::string PreprocessShader(const std::string_view& fileName, const std::string& source,
			shaderc_shader_kind shaderKind = shaderc_glsl_infer_from_source);
		std::string CompileToAssembly(const std::string_view& fileName, const std::string& source,
			shaderc_shader_kind shaderKind = shaderc_glsl_infer_from_source);
		std::vector<uint32_t> CompileFile(const std::string_view& fileName, const std::string& source,
			shaderc_shader_kind shaderKind = shaderc_glsl_infer_from_source);

		static std::string BaseFolder;
		static std::string BSDFFolder;

	private:
		// Helper classes taken from glslc: https://github.com/google/shaderc
		class FileFinder
		{
		public:
			std::string FindReadableFilepath(const std::string& filename) const;
			std::string FindRelativeReadableFilepath(const std::string& requesting_file, const std::string& filename) const;
			std::vector<std::string>& search_path() { return search_path_; }

		private:
			std::vector<std::string> search_path_;
		};
		class FileIncluder : public shaderc::CompileOptions::IncluderInterface
		{
		public:
			explicit FileIncluder(const FileFinder* file_finder) : file_finder_(*file_finder) {}

			~FileIncluder() override;
			shaderc_include_result* GetInclude(const char* requested_source,
				shaderc_include_type type,
				const char* requesting_source,
				size_t include_depth) override;
			void ReleaseInclude(shaderc_include_result* include_result) override;
			const std::unordered_set<std::string>& file_path_trace() const { return included_files_; }

		private:
			const FileFinder& file_finder_;
			struct FileInfo
			{
				const std::string full_path;
				std::vector<char> contents;
			};
			std::unordered_set<std::string> included_files_;
		};

		void InferKindAndStageFromFileName(const std::string_view& fileName);

		Context& m_context;
		shaderc::Compiler m_Compiler;
		shaderc::CompileOptions m_CompileOptions;
		FileFinder m_Finder{};
		VkShaderModule m_Module = nullptr;

		shaderc_shader_kind m_kind = shaderc_glsl_infer_from_source;
		VkShaderStageFlagBits m_vkStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM; // Default to an invalid stage

		static std::vector<char> ReadFile(const std::string_view& fileName);
		static std::string ReadTextFile(const std::string_view& fileName);
	};
}