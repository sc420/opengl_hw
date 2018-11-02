#pragma once

GLchar* LoadShaderSource(const std::string &file)
{
	FILE* fp;
	fopen_s(&fp, file.c_str(), "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	fclose(fp);
	return static_cast<GLchar*>(src);
}

void FreeShaderSource(GLchar* src)
{
	delete src;
}
