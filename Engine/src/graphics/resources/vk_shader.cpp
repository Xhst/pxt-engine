// Code based on https://github.com/jbikker/lighthouse2/blob/master/lib/rendercore_vulkan_rt/vulkan_shader.cpp
#include "graphics/resources/vk_shader.hpp"

inline std::string get_cwd() {
	char* cwd;
	cwd = new char[FILENAME_MAX * sizeof(char)];
#ifdef WIN32
	_getcwd(cwd, FILENAME_MAX);
#else
	getcwd(cwd, FILENAME_MAX);
#endif

	std::string cwdStr = std::string(cwd);
	delete[] cwd;
	return cwdStr;
}

namespace PXTEngine
{
	inline bool IsSPIR_V(const std::string_view& fileName)
	{
		const std::string_view extention = fileName.substr(fileName.size() - 4, 4);
		return extention == ".spv";
	}

	void VulkanShader::inferKindAndStageFromFileName(const std::string_view& fileName) {
		std::string_view effectiveFileName = fileName;

		// 1. Check if the file is a SPIR-V file (.spv)
		// If it is, we need to remove the ".spv" suffix before finding the actual shader stage extension.
		if (IsSPIR_V(fileName)) {
			// Create a new string_view that ends before ".spv"
			effectiveFileName = fileName.substr(0, fileName.length() - 4);
		}

		// 2. Find the last dot to extract the extension
		const auto dotIndex = effectiveFileName.find_last_of('.');

		if (dotIndex == std::string_view::npos) {
			// No dot found in effectiveFileName (e.g., "myshader" or "myshader.spv" that became "myshader")
			// In this case, 'extension' will remain an empty string_view, which will fall into the default case below.
			PXT_ERROR("Warning: No file extension found for: {}", fileName);
			throw;
			
		}

		// Extract the part after the last dot as the extension
		const auto extension = effectiveFileName.substr(dotIndex + 1);

		if (extension == "comp") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_compute_shader;
			m_vkStage = VK_SHADER_STAGE_COMPUTE_BIT;
		}
		else if (extension == "vert") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_vertex_shader;
			m_vkStage = VK_SHADER_STAGE_VERTEX_BIT;
		}
		else if (extension == "frag") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_fragment_shader;
			m_vkStage = VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		else if (extension == "geom") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_geometry_shader;
			m_vkStage = VK_SHADER_STAGE_GEOMETRY_BIT;
		}
		else if (extension == "mesh") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_mesh_shader;
			m_vkStage = VK_SHADER_STAGE_MESH_BIT_NV; // Note: VK_NV_mesh_shader extension is required
		}
		else if (extension == "tessc") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_tess_control_shader;
			m_vkStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		}
		else if (extension == "tesse") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_tess_evaluation_shader;
			m_vkStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		}
		else if (extension == "rchit") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_closesthit_shader;
			m_vkStage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR; // Part of VK_KHR_ray_tracing_pipeline
		}
		else if (extension == "rgen") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_raygen_shader;
			m_vkStage = VK_SHADER_STAGE_RAYGEN_BIT_KHR; // Part of VK_KHR_ray_tracing_pipeline
		}
		else if (extension == "rmiss") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_miss_shader;
			m_vkStage = VK_SHADER_STAGE_MISS_BIT_KHR; // Part of VK_KHR_ray_tracing_pipeline
		}
		else if (extension == "rahit") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_anyhit_shader;
			m_vkStage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR; // Part of VK_KHR_ray_tracing_pipeline
		}
		else if (extension == "rcall") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_callable_shader;
			m_vkStage = VK_SHADER_STAGE_CALLABLE_BIT_KHR; // Part of VK_KHR_ray_tracing_pipeline
		}
		else if (extension == "rint") {
			m_kind = shaderc_shader_kind::shaderc_glsl_default_intersection_shader;
			m_vkStage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR; // Part of VK_KHR_ray_tracing_pipeline
		}
		else {
			PXT_ERROR("Unrecognized shader extension {} for file: {}", extension, fileName);
		}
	}

	VulkanShader::VulkanShader(Context& context, const std::string_view& fileName,
		const std::vector<std::pair<std::string, std::string>>& definitions)
		: m_context(context) {
		const auto cwd = get_cwd();
		const std::string fileLocation = cwd + "/" + std::string(fileName.data());

		// Sanity check
		//if (!FileExists(fileLocation.c_str()))
			//PXT_FATAL("File: \"%s\" does not exist.", fileLocation.data());

		// Infer shader kind and stage from file name
		inferKindAndStageFromFileName(fileName);

		// Setup compiler environment
		m_compileOptions.SetIncluder(std::make_unique<FileIncluder>(&m_finder));
		m_compileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
		m_compileOptions.SetSourceLanguage(shaderc_source_language_glsl);
		//m_compileOptions.SetTargetSpirv(shaderc_spirv_version_1_4);
		m_compileOptions.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::vector<std::pair<std::string, std::string>> defs = definitions;

		for (auto& defPair : defs) // Add given definitions to compiler
			m_compileOptions.AddMacroDefinition(defPair.first, defPair.second);

		if (IsSPIR_V(fileName)) // In case we get fed an pre-compiled SPIR-V shader
		{
			const auto source = readFile(fileLocation);
			m_context.createShaderModuleFromSpirV(source, &m_module);
			PXT_ASSERT(m_module, "Could not create shader module for shader: \"%s\".", fileLocation.data());
		}
		else // We need to compile the shader ourselves
		{
			const std::string sourceString = readTextFile(fileLocation);			  // Get source of shader
			const auto result = preprocessShader(fileLocation, sourceString, m_kind); // Preprocess source file
			const auto binary = compileFile(fileLocation, result, m_kind);			  // Produce SPIR-V binary

			m_context.createShaderModuleFromSourceBinary(binary, &m_module);
			PXT_ASSERT(m_module, "Could not create shader module for shader: \"%s\".", fileLocation.data());
		}
	}

	VulkanShader::~VulkanShader() {
		cleanup();
	}

	void VulkanShader::cleanup() {
		if (m_module) {
			vkDestroyShaderModule(m_context.getDevice(), m_module, nullptr);
			m_module = nullptr;
		}
	}

	VkPipelineShaderStageCreateInfo VulkanShader::getShaderStageCreateInfo() {
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = m_vkStage;
		shaderStageInfo.module = m_module;
		shaderStageInfo.pName = "main";
		shaderStageInfo.flags = 0;
		shaderStageInfo.pNext = nullptr;
		shaderStageInfo.pSpecializationInfo = nullptr;

		return shaderStageInfo;
	}

	std::vector<char> VulkanShader::readFile(const std::string_view& fileName) {
		std::ifstream fileStream(fileName.data(), std::ios::binary | std::ios::in | std::ios::ate);
		if (!fileStream.is_open())
			PXT_FATAL("Could not open file.");

		const size_t size = fileStream.tellg();
		fileStream.seekg(0, std::ios::beg);
		std::vector<char> data(size);
		fileStream.read(data.data(), size);
		fileStream.close();
		return data;
	}

	std::string VulkanShader::readTextFile(const std::string_view& fileName) {
		std::string buffer;
		std::ifstream fileStream(fileName.data());
		if (!fileStream.is_open())
			PXT_FATAL("Could not open file.");

		std::string temp;
		while (getline(fileStream, temp))
			buffer.append(temp), buffer.append("\n");

		fileStream >> buffer;
		fileStream.close();
		return buffer;
	}

	// The shader compilation implementation below was taken from google/shaderc itself
	// It implements include support for GLSL
	// https://github.com/google/shaderc
	std::string VulkanShader::preprocessShader(const std::string_view& fileName, const std::string& source, shaderc_shader_kind shaderKind) {
		auto result = m_compiler.PreprocessGlsl(source, shaderKind, fileName.data(), m_compileOptions);
		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			PXT_FATAL("%s", result.GetErrorMessage().data());
			return "";
		}
		return { result.cbegin(), result.cend() };
	}

	std::string VulkanShader::compileToAssembly(const std::string_view& fileName, const std::string& source, shaderc_shader_kind shaderKind) {
		auto result = m_compiler.CompileGlslToSpvAssembly(source, shaderKind, fileName.data(), m_compileOptions);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			PXT_FATAL("%s", result.GetErrorMessage().data());
			return "";
		}
		return { result.cbegin(), result.cend() };
	}

	std::vector<uint32_t> VulkanShader::compileFile(const std::string_view& fileName, const std::string& source, shaderc_shader_kind shaderKind) {
		auto module = m_compiler.CompileGlslToSpv(source, shaderKind, fileName.data(), m_compileOptions);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			PXT_FATAL("%s", module.GetErrorMessage().data());
			return std::vector<uint32_t>();
		}

		return { module.cbegin(), module.cend() };
	}

	shaderc_include_result* MakeErrorIncludeResult(const char* message) {
		return new shaderc_include_result{ "", 0, message, strlen(message) };
	}

	class string_piece {
	public:
		typedef const char* iterator;
		static const size_t npos = -1;

		string_piece() {}

		string_piece(const char* begin, const char* end) : begin_(begin), end_(end)
		{
			assert((begin == nullptr) == (end == nullptr) &&
				"either both begin and end must be nullptr or neither must be");
		}

		string_piece(const char* string) : begin_(string), end_(string)
		{
			if (string)
			{
				end_ += strlen(string);
			}
		}

		string_piece(const std::string& str)
		{
			if (!str.empty())
			{
				begin_ = &(str.front());
				end_ = &(str.back()) + 1;
			}
		}

		string_piece(const string_piece& other)
		{
			begin_ = other.begin_;
			end_ = other.end_;
		}

		void clear()
		{
			begin_ = nullptr;
			end_ = nullptr;
		}

		const char* data() const { return begin_; }

		std::string str() const { return std::string(begin_, end_); }

		string_piece substr(size_t pos, size_t len = npos) const
		{
			assert(len == npos || pos + len <= size());
			return string_piece(begin_ + pos, len == npos ? end_ : begin_ + pos + len);
		}

		template <typename T>
		size_t find_first_not_matching(T callee)
		{
			for (auto it = begin_; it != end_; ++it)
			{
				if (!callee(*it))
				{
					return it - begin_;
				}
			}
			return npos;
		}

		size_t find_first_not_of(const string_piece& to_search,
			size_t pos = 0) const
		{
			if (pos >= size())
			{
				return npos;
			}
			for (auto it = begin_ + pos; it != end_; ++it)
			{
				if (to_search.find_first_of(*it) == npos)
				{
					return it - begin_;
				}
			}
			return npos;
		}

		size_t find_first_not_of(char to_search, size_t pos = 0) const
		{
			return find_first_not_of(string_piece(&to_search, &to_search + 1), pos);
		}

		size_t find_first_of(const string_piece& to_search, size_t pos = 0) const
		{
			if (pos >= size())
			{
				return npos;
			}
			for (auto it = begin_ + pos; it != end_; ++it)
			{
				for (char c : to_search)
				{
					if (c == *it)
					{
						return it - begin_;
					}
				}
			}
			return npos;
		}

		size_t find_first_of(char to_search, size_t pos = 0) const
		{
			return find_first_of(string_piece(&to_search, &to_search + 1), pos);
		}

		size_t find_last_of(const string_piece& to_search, size_t pos = npos) const
		{
			if (empty()) return npos;
			if (pos >= size())
			{
				pos = size();
			}
			auto it = begin_ + pos + 1;
			do
			{
				--it;
				if (to_search.find_first_of(*it) != npos)
				{
					return it - begin_;
				}
			} while (it != begin_);
			return npos;
		}

		size_t find_last_of(char to_search, size_t pos = npos) const
		{
			return find_last_of(string_piece(&to_search, &to_search + 1), pos);
		}

		size_t find_last_not_of(const string_piece& to_search,
			size_t pos = npos) const
		{
			if (empty()) return npos;
			if (pos >= size())
			{
				pos = size();
			}
			auto it = begin_ + pos + 1;
			do
			{
				--it;
				if (to_search.find_first_of(*it) == npos)
				{
					return it - begin_;
				}
			} while (it != begin_);
			return npos;
		}

		size_t find_last_not_of(char to_search, size_t pos = 0) const
		{
			return find_last_not_of(string_piece(&to_search, &to_search + 1), pos);
		}

		string_piece lstrip(const string_piece& chars_to_strip) const
		{
			iterator begin = begin_;
			for (; begin < end_; ++begin)
				if (chars_to_strip.find_first_of(*begin) == npos) break;
			if (begin >= end_) return string_piece();
			return string_piece(begin, end_);
		}

		string_piece rstrip(const string_piece& chars_to_strip) const
		{
			iterator end = end_;
			for (; begin_ < end; --end)
				if (chars_to_strip.find_first_of(*(end - 1)) == npos) break;
			if (begin_ >= end) return string_piece();
			return string_piece(begin_, end);
		}

		string_piece strip(const string_piece& chars_to_strip) const { return lstrip(chars_to_strip).rstrip(chars_to_strip); }
		string_piece strip_whitespace() const { return strip(" \t\n\r\f\v"); }
		const char& operator[](size_t i) const { return *(begin_ + i); }
		bool operator==(const string_piece& other) const
		{
			// Either end_ and _begin_ are nullptr or neither of them are.
			assert(((end_ == nullptr) == (begin_ == nullptr)));
			assert(((other.end_ == nullptr) == (other.begin_ == nullptr)));
			if (size() != other.size())
			{
				return false;
			}
			return (memcmp(begin_, other.begin_, end_ - begin_) == 0);
		}

		bool operator!=(const string_piece& other) const
		{
			return !operator==(other);
		}

		iterator begin() const { return begin_; }
		iterator end() const { return end_; }

		const char& front() const
		{
			assert(!empty());
			return *begin_;
		}

		const char& back() const
		{
			assert(!empty());
			return *(end_ - 1);
		}

		bool starts_with(const string_piece& other) const
		{
			const char* iter = begin_;
			const char* other_iter = other.begin();
			while (iter != end_ && other_iter != other.end())
			{
				if (*iter++ != *other_iter++)
				{
					return false;
				}
			}
			return other_iter == other.end();
		}

		size_t find(const string_piece& substr, size_t pos = 0) const
		{
			if (empty()) return npos;
			if (pos >= size()) return npos;
			if (substr.empty()) return 0;
			for (auto it = begin_ + pos;
				end() - it >= static_cast<decltype(end() - it)>(substr.size()); ++it)
			{
				if (string_piece(it, end()).starts_with(substr)) return it - begin_;
			}
			return npos;
		}
		size_t find(char character, size_t pos = 0) const { return find_first_of(character, pos); }
		bool empty() const { return begin_ == end_; }
		size_t size() const { return end_ - begin_; }
		std::vector<string_piece> get_fields(char delimiter,
			bool keep_delimiter = false) const
		{
			std::vector<string_piece> fields;
			size_t first = 0;
			size_t field_break = find_first_of(delimiter);
			while (field_break != npos)
			{
				fields.push_back(substr(first, field_break - first + keep_delimiter));
				first = field_break + 1;
				field_break = find_first_of(delimiter, first);
			}
			if (size() - first > 0)
			{
				fields.push_back(substr(first, size() - first));
			}
			return fields;
		}

		friend std::ostream& operator<<(std::ostream& os, const string_piece& piece);

	private:
		// It is expected that begin_ and end_ will both be null or
		// they will both point to valid pieces of memory, but it is invalid
		// to have one of them being nullptr and the other not.
		string_piece::iterator begin_ = nullptr;
		string_piece::iterator end_ = nullptr;
	};

	inline std::ostream& operator<<(std::ostream& os, const string_piece& piece)
	{
		// Either end_ and _begin_ are nullptr or neither of them are.
		assert(((piece.end_ == nullptr) == (piece.begin_ == nullptr)));
		if (piece.end_ != piece.begin_)
		{
			os.write(piece.begin_, piece.end_ - piece.begin_);
		}
		return os;
	}

	inline bool operator==(const char* first, const string_piece second)
	{
		return second == first;
	}

	inline bool operator!=(const char* first, const string_piece second)
	{
		return !operator==(first, second);
	}

	// Returns "" if path is empty or ends in '/'.  Otherwise, returns "/".
	std::string MaybeSlash(const string_piece& path)
	{
		return (path.empty() || path.back() == '/') ? "" : "/";
	}

	std::string VulkanShader::FileFinder::findReadableFilepath(
		const std::string& filename) const
	{
		assert(!filename.empty());
		static const auto for_reading = std::ios_base::in;
		std::filebuf opener;
		for (const auto& prefix : m_searchPath)
		{
			const std::string prefixed_filename =
				prefix + MaybeSlash(prefix) + filename;
			if (opener.open(prefixed_filename, for_reading)) return prefixed_filename;
		}
		return "";
	}

	std::string VulkanShader::FileFinder::findRelativeReadableFilepath(
		const std::string& requesting_file, const std::string& filename) const
	{
		assert(!filename.empty());

		string_piece dir_name(requesting_file);

		size_t last_slash = requesting_file.find_last_of("/\\");
		if (last_slash != std::string::npos)
		{
			dir_name = string_piece(requesting_file.c_str(),
				requesting_file.c_str() + last_slash);
		}

		if (dir_name.size() == requesting_file.size())
		{
			dir_name.clear();
		}

		static const auto for_reading = std::ios_base::in;
		std::filebuf opener;
		const std::string relative_filename =
			dir_name.str() + MaybeSlash(dir_name) + filename;
		if (opener.open(relative_filename, for_reading)) return relative_filename;

		return findReadableFilepath(filename);
	}

	VulkanShader::FileIncluder::~FileIncluder() = default;

	shaderc_include_result* VulkanShader::FileIncluder::GetInclude(
		const char* requested_source, shaderc_include_type include_type,
		const char* requesting_source, size_t)
	{

		const std::string full_path =
			(include_type == shaderc_include_type_relative)
			? m_fileFinder.findRelativeReadableFilepath(requesting_source,
				requested_source)
			: m_fileFinder.findReadableFilepath(requested_source);

		if (full_path.empty())
			return MakeErrorIncludeResult("Cannot find or open include file.");

		// In principle, several threads could be resolving includes at the same
		// time.  Protect the included_files.

		const auto ReadFile = [](const std::string& input_file_name,
			std::vector<char>* input_data) -> bool {
				std::istream* stream = &std::cin;
				std::ifstream input_file;
				if (input_file_name != "-")
				{
					input_file.open(input_file_name, std::ios_base::binary);
					stream = &input_file;
					if (input_file.fail())
					{
						std::string errorMessage = std::string("Cannot open input file: ") + input_file_name;
						PXT_FATAL("%s", errorMessage.data());
						return false;
					}
				}
				*input_data = std::vector<char>((std::istreambuf_iterator<char>(*stream)),
					std::istreambuf_iterator<char>());
				return true;
			};

		// Read the file and save its full path and contents into stable addresses.
		FileInfo* new_file_info = new FileInfo{ full_path, {} };
		if (!ReadFile(full_path, &(new_file_info->contents)))
		{
			return MakeErrorIncludeResult("Cannot read file");
		}

		m_includedFiles.insert(full_path);

		return new shaderc_include_result{
			new_file_info->fullPath.data(), new_file_info->fullPath.length(),
			new_file_info->contents.data(), new_file_info->contents.size(),
			new_file_info };
	}

	void VulkanShader::FileIncluder::ReleaseInclude(shaderc_include_result* include_result) {
		FileInfo* info = static_cast<FileInfo*>(include_result->user_data);
		delete info;
		delete include_result;
	}
}