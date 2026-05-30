project(SRB2XMAS)

cmake_minimum_required(VERSION 3.17.2)

if (EMSCRIPTEN)
    set(SDL2_MIXER_LIBRARIES "${CMAKE_SOURCE_DIR}/emlibs/libSDL2_mixer.a")
else()
    find_package(SDL2 REQUIRED)
    find_package(SDL2_mixer REQUIRED)
endif()

set(TOTAL_SOURCES
    am_map.c
    command.c
    console.c
    d_clisrv.c
    d_items.c
    d_main.c
    d_net.c
    d_netcmd.c
    d_netfil.c
    dehacked.c
    dstrings.c
    f_finale.c
    f_wipe.c
    g_game.c
    g_input.c
    g_state.c
    hu_stuff.c
    i_tcp.c
    info.c
    m_argv.c
    m_bbox.c
    m_cheat.c
    m_fixed.c
    m_menu.c
    m_misc.c
    m_random.c
    m_swap.c
    p_ceilng.c
    p_doors.c
    p_enemy.c
    p_fab.c
    p_floor.c
    p_inter.c
    p_lights.c
    p_map.c
    p_maputl.c
    p_mobj.c
    p_plats.c
    p_pspr.c
    p_saveg.c
    p_setup.c
    p_sight.c
    p_spec.c
    p_switch.c
    p_telept.c
    p_tick.c
    p_user.c
    qmus2mid.c
    r_bsp.c
    r_data.c
    r_draw.c
    r_main.c
    r_plane.c
    r_segs.c
    r_sky.c
    r_splats.c
    r_things.c
    s_sound.c
    screen.c
    sounds.c
    st_lib.c
    st_stuff.c
    tables.c
    v_video.c
    w_wad.c
    wi_stuff.c
    z_zone.c

    # now for the interface
    sdl/i_cdmus.c
    sdl/i_main.c
    sdl/i_net.c
    sdl/i_sound.c
    sdl/i_system.c
    sdl/i_video.c
    sdl/dosstr.c
)

add_executable(SRB2SDLXMAS ${TOTAL_SOURCES})
target_compile_definitions(SRB2SDLXMAS PRIVATE LINUX)
target_link_libraries(SRB2SDLXMAS PRIVATE ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARIES} m)

if (EMSCRIPTEN)
	add_subdirectory(web)
endif()
