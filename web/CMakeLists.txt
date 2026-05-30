file(COPY
	${PROJECT_SOURCE_DIR}/web/index.html
	${PROJECT_SOURCE_DIR}/web/favicon.ico
	${PROJECT_SOURCE_DIR}/web/timidity.data
	${PROJECT_SOURCE_DIR}/web/js/timidity.js
	DESTINATION ${PROJECT_BINARY_DIR}
)

set_target_properties(SRB2SDLXMAS PROPERTIES OUTPUT_NAME "SRB2SDLXMAS" SUFFIX ".html")
target_link_options(SRB2SDLXMAS PRIVATE
	--use-port=sdl2
	--use-port=sdl2_mixer
	-sEXPORTED_RUNTIME_METHODS=ccall,cwrap
	-sEXPORTED_FUNCTIONS=_main,_P_AddWadFile
	-sNO_EXIT_RUNTIME
	-sALLOW_MEMORY_GROWTH=1
	-sINITIAL_MEMORY=983040000
	-sSTACK_SIZE=655360000
	--preload-file=${PROJECT_SOURCE_DIR}/assets/installer@/
	--use-preload-plugins
	--use-preload-cache
	-sFORCE_FILESYSTEM=1
)
target_compile_options(SRB2SDLXMAS PRIVATE --use-port=sdl2 --use-port=sdl2_mixer)
target_compile_definitions(SRB2SDLXMAS PRIVATE __EMSCRIPTEN__)
