@echo off
set "UV_CACHE_DIR=C:\Users\dnjs4475\Documents\UnrealProjects\Project-SX\.uv-cache-mcp"
"C:\Users\dnjs4475\.local\bin\uv.exe" --directory "C:\Users\dnjs4475\Documents\UnrealProjects\Project-SX\Tools\unreal-engine-mcp\Python" run --no-project --with "mcp[cli]>=1.4.1" --with "fastmcp>=0.2.0" --with uvicorn --with fastapi --with "pydantic>=2.6.1" --with requests python unreal_mcp_server_advanced.py
