#define AVG_ITEM_SIZE			420

#define QUERY_XMB(key, src) 	"<Query class=\"type:x-xmb/folder-pixmap\" key=\"" key "\" attr=\"" key "\" src=\"" src "\"/>"
#define ADD_XMB_ITEM(key)		"<Item class=\"type:x-xmb/module-action\" key=\"" key "\" attr=\"" key "\"/>"

#define XML_HEADER				"<?xml version=\"1.0\" encoding=\"UTF-8\"?><XMBML version=\"1.0\">"
#define XML_PAIR(key, value) 	"<Pair key=\"" key "\"><String>" value "</String></Pair>"

#define XAI_LINK_PAIR			XML_PAIR("module_name", WM_PROXY_SPRX) XML_PAIR("bar_action", "none")
#define WEB_LINK_PAIR			XML_PAIR("module_name", "webbrowser_plugin")

#define STR_NOITEM_PAIR			XML_PAIR("str_noitem", "msg_error_no_content") "</Table>"

#define XML_KEY_LEN 7

#define USE_MC      99

typedef struct
{
	char value[12];
} t_keys;

enum xmb_groups
{
	gPS3 = 0,
	gPSX = 1,
	gPS2 = 2,
	gPSP = 3,
	gDVD = 4,
	gROM = 5,
};

static void refresh_xml(char *msg)
{
	setPluginActive();

	refreshing_xml = 1;
	webman_config->profile = profile; save_settings();

	sprintf(msg, "%s XML%s: %s", STR_REFRESH, SUFIX2(profile), STR_SCAN2);
	show_msg(msg);

	sys_ppu_thread_t t_id;
	sys_ppu_thread_create(&t_id, handleclient_www, (u64)REFRESH_CONTENT, THREAD_PRIO_HIGH, THREAD_STACK_SIZE_WEB_CLIENT, SYS_PPU_THREAD_CREATE_NORMAL, THREAD_NAME_CMD);

	while(refreshing_xml && working) sys_ppu_thread_usleep(200000);

	sprintf(msg, "%s XML%s: OK", STR_REFRESH, SUFIX2(profile));
	show_msg(msg);

	setPluginInactive();
}

static bool add_xmb_entry(u8 f0, u8 f1, int plen, char *tempstr, char *templn, char *skey, u16 key, char *myxml_ps3, char *myxml_ps2, char *myxml_psx, char *myxml_psp, char *myxml_dvd, char *myxml_roms, char *entry_name, u16 *item_count, u32 *xml_len, u8 subfolder)
{
	set_sort_key(skey, templn, key, subfolder, f1);

	#define ITEMS_BUFFER(a)  (64 * (item_count[a] + 8))

	if( !(webman_config->nogrp) )
	{
 #ifdef MOUNT_ROMS
		if((IS_ROMS_FOLDER) && xml_len[gROM] < (BUFFER_SIZE_ROM - ITEMS_BUFFER(gROM)))
		{xml_len[gROM] += concat(myxml_roms + xml_len[gROM], tempstr); *skey=DVD, ++item_count[gROM];}
		else
 #endif
#ifdef COBRA_ONLY
		if(((IS_PS3_TYPE)   || ((IS_NTFS) && !extcmp(entry_name + plen, ".ntfs[PS3ISO]", 13))) && xml_len[gPS3] < (BUFFER_SIZE - _4KB_ - ITEMS_BUFFER(gPS3)))
		{xml_len[gPS3] += concat(myxml_ps3 + xml_len[gPS3], tempstr); *skey=PS3_, ++item_count[gPS3];}
		else
		if(((IS_PS2_FOLDER) || ((IS_NTFS) && !extcmp(entry_name + plen, ".ntfs[PS2ISO]", 13))) && xml_len[gPS2] < (BUFFER_SIZE_PS2 - ITEMS_BUFFER(gPS2)))
		{xml_len[gPS2] += concat(myxml_ps2 + xml_len[gPS2], tempstr); *skey=PS2, ++item_count[gPS2];}
		else
		if(((IS_PSX_FOLDER) || ((IS_NTFS) && !extcmp(entry_name + plen, ".ntfs[PSXISO]", 13))) && xml_len[gPSX] < (BUFFER_SIZE_PSX - ITEMS_BUFFER(gPSX)))
		{xml_len[gPSX] += concat(myxml_psx + xml_len[gPSX], tempstr); *skey=PS1, ++item_count[gPSX];}
		else
		if(((IS_PSP_FOLDER) || ((IS_NTFS) && !extcmp(entry_name + plen, ".ntfs[PSPISO]", 13))) && xml_len[gPSP] < (BUFFER_SIZE_PSP - ITEMS_BUFFER(gPSP)))
		{xml_len[gPSP] += concat(myxml_psp + xml_len[gPSP], tempstr); *skey=PSP, ++item_count[gPSP];}
		else
		if(((IS_BLU_FOLDER) || (IS_DVD_FOLDER) || ((IS_NTFS) && (!extcmp(entry_name + plen, ".ntfs[DVDISO]", 13) || !extcmp(entry_name + plen, ".ntfs[BDISO]", 12) || !extcmp(entry_name + plen, ".ntfs[BDFILE]", 13)))) && xml_len[gDVD] < (BUFFER_SIZE_DVD - ITEMS_BUFFER(gDVD)))
		{xml_len[gDVD] += concat(myxml_dvd + xml_len[gDVD], tempstr); *skey=BLU, ++item_count[gDVD];}
#else
		if((IS_PS3_TYPE) && xml_len[gPS3] < (BUFFER_SIZE - _4KB_ - ITEMS_BUFFER(gPS3)))
		{xml_len[gPS3] += concat(myxml_ps3 + xml_len[gPS3], tempstr); *skey=PS3_, ++item_count[gPS3];}
		else
		if((IS_PS2_FOLDER) && xml_len[gPS2] < (BUFFER_SIZE_PS2 - ITEMS_BUFFER(gPS2)))
		{xml_len[gPS2] += concat(myxml_ps2 + xml_len[gPS2], tempstr); *skey=PS2, ++item_count[gPS2];}
#endif
		else
			return (false);
	}
	else
	{
		if(xml_len[gPS3] < (BUFFER_SIZE  - ITEMS_BUFFER(gPS3) - _4KB_))
			{xml_len[gPS3] += concat(myxml_ps3+xml_len[gPS3], tempstr); ++item_count[gPS3];}
		else
			return (false);
	}

	return (true);
}

static void make_fb_xml(char *myxml)
{
 #ifndef ENGLISH_ONLY
	char STR_LOADGAMES[72];//	= "Load games with webMAN";

	language("STR_LOADGAMES", STR_LOADGAMES, "Load games with webMAN");
	close_language();
 #endif

	u16 size = sprintf(myxml, "%s"
							  "<View id=\"seg_fb\">"
							  "<Attributes><Table key=\"mgames\">%s"
//							  XML_PAIR("icon_notation","WNT_XmbItemSavePS3")
							  XML_PAIR("ingame","disable")
							  XML_PAIR("title","%s%s")
							  XML_PAIR("info","%s")
							  "</Table>"
							  "%s"
							  QUERY_XMB("mgames", "xmb://localhost%s#seg_mygames")
							  "%s</XMBML>", XML_HEADER,
							  file_exists(WM_ICONS_PATH "/icon_wm_root.png") ?
									XML_PAIR("icon", WM_ICONS_PATH "/icon_wm_root.png") :
									XML_PAIR("icon_rsc", "item_tex_ps3util"),
							  STR_MYGAMES, SUFIX2(profile),
							  STR_LOADGAMES,
							  "</Attributes><Items>", MY_GAMES_XML, "</Items></View>");

	save_file(FB_XML, myxml, size);
}

static u32 get_buffer_size(u8 footprint)
{
	if((webman_config->mc_app == 0) && (footprint == USE_MC)) //mc_app
	{
		return _3MB_;
	}

	if(footprint == 1) //MIN
	{
#ifndef LITE_EDITION
		return ( 320*KB);
#else
		return ( _256KB_);
#endif
	}
	else
	if(footprint == 2 || footprint >= 4) //MAX
	{
		return ( 1280*KB);
	}
	else
	if(footprint == 3) //MIN+
	{
		return (_512KB_);
	}
	else	//STANDARD
	{
		return ( 896*KB);
	}
}

static void set_buffer_sizes(u8 footprint)
{
	BUFFER_SIZE_ALL = get_buffer_size(footprint);
	BUFFER_SIZE_FTP	= ( _128KB_);

	BUFFER_SIZE_PSP	= (  _32KB_);
	BUFFER_SIZE_PS2	= (  _64KB_);
	BUFFER_SIZE_DVD	= (  _64KB_);

	if(footprint == USE_MC) //mc_app
	{
		BUFFER_SIZE_FTP	= ( _128KB_);

		//BUFFER_SIZE	= (1792*KB);
		BUFFER_SIZE_PSX	= (_384KB_);
		BUFFER_SIZE_PSP	= (_128KB_);
		BUFFER_SIZE_PS2	= (_256KB_);
		BUFFER_SIZE_DVD	= (_512KB_);
	}
	else
	if(footprint == 1) //MIN
	{
		//BUFFER_SIZE	= ( _128KB_);
		BUFFER_SIZE_PSX	= (  _32KB_);
	}
	else
	if(footprint == 2) //MAX
	{
		BUFFER_SIZE_FTP	= ( _256KB_);
		//BUFFER_SIZE	= ( _512KB_);
		BUFFER_SIZE_PSX	= ( _256KB_);
		BUFFER_SIZE_PSP	= (  _64KB_);
		BUFFER_SIZE_PS2	= ( _128KB_);
		BUFFER_SIZE_DVD	= ( _192KB_);
	}
	else
	if(footprint == 3) //MIN+
	{
		//BUFFER_SIZE	= ( 320*KB);
		BUFFER_SIZE_PSX	= (  _32KB_);
	}
	else
	if(footprint == 4) //MAX PS3+
	{
		//BUFFER_SIZE	= ( 1088*KB);
		BUFFER_SIZE_PSX	= (  _32KB_);
	}
	else
	if(footprint == 5) //MAX PSX+
	{
		//BUFFER_SIZE	= (  368*KB);
		BUFFER_SIZE_PSX	= (  768*KB);
		BUFFER_SIZE_PSP	= (  _64KB_);
	}
	else
	if(footprint == 6) //MAX BLU+
	{
		//BUFFER_SIZE	= (  368*KB);
		BUFFER_SIZE_PSX	= (  _64KB_);
		BUFFER_SIZE_PSP	= (  _64KB_);
		BUFFER_SIZE_DVD	= (  768*KB);
	}
	else
	if(footprint == 7) //MAX PSP+
	{
		//BUFFER_SIZE	= (  368*KB);
		BUFFER_SIZE_PSX	= (  _64KB_);
#ifdef MOUNT_ROMS
		BUFFER_SIZE_PSP	= (webman_config->roms) ? ( _128KB_) : (768*KB);
#else
		BUFFER_SIZE_PSP	= (768*KB);
#endif
		BUFFER_SIZE_DVD	= (  _64KB_);
	}
	else	// if(footprint == 0) STANDARD
	{
		BUFFER_SIZE_ALL = ( 896*KB);
		//BUFFER_SIZE	= ( 448*KB);
		BUFFER_SIZE_PSX	= ( 160*KB);
		BUFFER_SIZE_DVD	= ( _192KB_);
	}

	if((webman_config->cmask & PS1)) BUFFER_SIZE_PSX	= (_8KB_);
	if((webman_config->cmask & PS2)) BUFFER_SIZE_PS2	= (_8KB_);
	if((webman_config->cmask & PSP)) BUFFER_SIZE_PSP	= (_8KB_);
	if((webman_config->cmask & (BLU | DVD)) == (BLU | DVD)) BUFFER_SIZE_DVD = (_8KB_);

#ifdef MOUNT_ROMS
	BUFFER_SIZE_ROM = !(webman_config->roms) ? _8KB_ : (footprint >= 7) ? _640KB_ : _64KB_;

	BUFFER_SIZE = BUFFER_SIZE_ALL - (BUFFER_SIZE_PSX + BUFFER_SIZE_PSP + BUFFER_SIZE_PS2 + BUFFER_SIZE_DVD + BUFFER_SIZE_ROM);
#else
	BUFFER_SIZE = BUFFER_SIZE_ALL - (BUFFER_SIZE_PSX + BUFFER_SIZE_PSP + BUFFER_SIZE_PS2 + BUFFER_SIZE_DVD);
#endif
}

static bool scan_mygames_xml(u64 conn_s_p)
{
	if(conn_s_p == START_DAEMON)
	{
		if((webman_config->refr==1) || from_reboot)
		{
			cellFsUnlink(WMNOSCAN);

			if(file_exists(MY_GAMES_XML))
			{
				char tmpxml[0x320];
				make_fb_xml(tmpxml);
				return true; // skip refresh xml & mount autoboot
			}
		}

		// start a new thread for refresh xml content at start up
		if(!webman_config->refr || file_exists(MY_GAMES_XML) == false)
		{
			sys_ppu_thread_t t_id;
			sys_ppu_thread_create(&t_id, handleclient_www, (u64)REFRESH_CONTENT, THREAD_PRIO, THREAD_STACK_SIZE_WEB_CLIENT, SYS_PPU_THREAD_CREATE_NORMAL, THREAD_NAME_CMD);
		}

		return true; // mount autoboot & refresh xml
	}

	sys_addr_t sysmem = NULL;

#ifdef USE_VM
	if(sys_vm_memory_map(_32MB_, _1MB_, SYS_MEMORY_CONTAINER_ID_INVALID, SYS_MEMORY_PAGE_SIZE_64K, SYS_VM_POLICY_AUTO_RECOMMENDED, &sysmem) != CELL_OK)
	{
		return false;  //leave if cannot allocate memory
	}
#else
	if(!webman_config->mc_app)
	{
		sys_memory_container_t mc_app = get_app_memory_container();
		if(mc_app && sys_memory_allocate_from_container(_3MB_, mc_app, SYS_MEMORY_PAGE_SIZE_1M, &sysmem) == CELL_OK) set_buffer_sizes(USE_MC);
	}

	if(!sysmem)
	{
		_meminfo meminfo;
		set_buffer_sizes(webman_config->foot);

		{system_call_1(SC_GET_FREE_MEM, (u64)(u32) &meminfo);}
		if( meminfo.avail<(BUFFER_SIZE_ALL+MIN_MEM)) set_buffer_sizes(3); //MIN+
		if (meminfo.avail<(BUFFER_SIZE_ALL+MIN_MEM)) set_buffer_sizes(1); //MIN
		if((meminfo.avail<(BUFFER_SIZE_ALL+MIN_MEM)) || sys_memory_allocate((BUFFER_SIZE_ALL), SYS_MEMORY_PAGE_SIZE_64K, &sysmem) != CELL_OK)
		{
			return false;  //leave if cannot allocate memory
		}
	}
#endif

	sys_addr_t sysmem_psx = sysmem + (BUFFER_SIZE);
	sys_addr_t sysmem_psp = sysmem + (BUFFER_SIZE) + (BUFFER_SIZE_PSX);
	sys_addr_t sysmem_ps2 = sysmem + (BUFFER_SIZE) + (BUFFER_SIZE_PSX) + (BUFFER_SIZE_PSP);
	sys_addr_t sysmem_dvd = sysmem + (BUFFER_SIZE) + (BUFFER_SIZE_PSX) + (BUFFER_SIZE_PSP) + (BUFFER_SIZE_PS2);

#ifdef MOUNT_ROMS
	sys_addr_t sysmem_rom = sysmem + (BUFFER_SIZE) + (BUFFER_SIZE_PSX) + (BUFFER_SIZE_PSP) + (BUFFER_SIZE_PS2) + (BUFFER_SIZE_DVD);
	char *myxml_roms  = (char*)sysmem_rom;
#else
	char *myxml_roms  = NULL;
#endif

	char *myxml_ps3   = (char*)sysmem;
	char *myxml_psx   = (char*)sysmem_psx;
	char *myxml_ps2   = (char*)sysmem_ps2;
	char *myxml_psp   = (char*)sysmem_psp;
	char *myxml_dvd   = (char*)sysmem_dvd;
	char *myxml       = (char*)sysmem+(BUFFER_SIZE)-4300;
	char *myxml_items = (char*)sysmem_dvd;

	cellFsMkdir("/dev_hdd0/xmlhost", MODE);
	cellFsMkdir("/dev_hdd0/xmlhost/game_plugin", MODE);

	u16 key = 0; const u16 max_xmb_items = ((u16)(BUFFER_SIZE_ALL / AVG_ITEM_SIZE));

	make_fb_xml(myxml);

	char templn[1024];

	// --- build group headers ---
	char *tempstr, *folder_name, *url; tempstr = myxml; memset(tempstr, 0, _4KB_); folder_name = myxml + (3*KB), url = myxml + _2KB_;

	u16 item_count[6]; u32 xml_len[6];
	for(u8 i = 0; i < 6; i++) item_count[i] = xml_len[i] = 0;

	cellFsUnlink(MY_GAMES_XML);

	key = 0;

	if(!(webman_config->nogrp))
	{
		if(!(webman_config->cmask & PS3))
		{
			xml_len[gPS3] = sprintf(myxml_ps3, "<View id=\"seg_wm_ps3_items\"><Attributes>");
#ifdef PKG_LAUNCHER
			if(webman_config->ps3l && file_exists("/dev_hdd0/game/PKGLAUNCH"))
			{
				sprintf(templn, "<Table key=\"pkg_launcher\">"
								XML_PAIR("icon","/dev_hdd0/game/PKGLAUNCH/ICON0.PNG")
								XML_PAIR("title","PKG Launcher")
								XML_PAIR("info","%s") "%s",
								"PKG Launcher", "</Table>"); xml_len[gPS3] += concat(myxml_ps3, templn);
			}
#endif
		}
		if(!(webman_config->cmask & PS2))
		{
			xml_len[gPS2] =  sprintf(myxml_ps2, "<View id=\"seg_wm_ps2_items\"><Attributes>");
			if(webman_config->ps2l && file_exists("/dev_hdd0/game/PS2U10000"))
			{
#ifndef ENGLISH_ONLY
				char *STR_LAUNCHPS2 =  tempstr; //[48];//	= "Launch PS2 Classic";
				language("STR_LAUNCHPS2", STR_LAUNCHPS2, "Launch PS2 Classic");
#endif
				sprintf(templn, "<Table key=\"ps2_classic_launcher\">"
								XML_PAIR("icon","/dev_hdd0/game/PS2U10000/ICON0.PNG")
								XML_PAIR("title","PS2 Classic Launcher")
								XML_PAIR("info","%s") "%s",
								STR_LAUNCHPS2, "</Table>"); xml_len[gPS2] += concat(myxml_ps2, templn);
			}
		}
#ifdef COBRA_ONLY
		if(!(webman_config->cmask & PS1)) {xml_len[gPSX] = sprintf(myxml_psx, "<View id=\"seg_wm_psx_items\"><Attributes>");}
		if(!(webman_config->cmask & PSP))
		{
			xml_len[gPSP] =  sprintf(myxml_psp, "<View id=\"seg_wm_psp_items\"><Attributes>");
			if(webman_config->pspl && (isDir("/dev_hdd0/game/PSPC66820") || isDir("/dev_hdd0/game/PSPM66820")))
			{
#ifndef ENGLISH_ONLY
				char *STR_LAUNCHPSP =  tempstr; //[144];//	= "Launch PSP ISO mounted through webMAN or mmCM";
				language("STR_LAUNCHPSP", STR_LAUNCHPSP, "Launch PSP ISO mounted through webMAN or mmCM");
#endif
				sprintf(templn, "<Table key=\"cobra_psp_launcher\">"
								XML_PAIR("icon","/dev_hdd0/game/%s/ICON0.PNG")
								XML_PAIR("title","PSP Launcher")
								XML_PAIR("info","%s") "%s",
								(isDir("/dev_hdd0/game/PSPC66820")) ? "PSPC66820" : "PSPM66820",
								STR_LAUNCHPSP, "</Table>"); xml_len[gPSP] += concat(myxml_psp, templn);
			}
		}
		if(!(webman_config->cmask & DVD) || !(webman_config->cmask & BLU))
		{
			xml_len[gDVD] = sprintf(myxml_dvd, "<View id=\"seg_wm_dvd_items\"><Attributes>");
			if(webman_config->rxvid)
			{
				sprintf(templn, "<Table key=\"rx_video\">"
								XML_PAIR("icon","%s")
								XML_PAIR("title","%s")
								XML_PAIR("child","segment") "%s",
								wm_icons[gDVD], STR_VIDLG, STR_NOITEM_PAIR); xml_len[gDVD] += concat(myxml_dvd, templn);
			}
		}
 #ifdef MOUNT_ROMS
		if(webman_config->roms)
		{
			xml_len[gROM] =  sprintf(myxml_roms, "<View id=\"seg_wm_rom_items\"><Attributes>");
		}
 #endif
#endif
	}

	int fd;
	t_keys skey[max_xmb_items];

	char param[STD_PATH_LEN], icon[STD_PATH_LEN], subpath[STD_PATH_LEN], enc_dir_name[1024];

	u8 is_net = 0;
	int abort_connection = 0;

	// --- scan xml content ---

	led(YELLOW, BLINK_FAST);

	check_cover_folders(tempstr);

#ifdef WM_PROXY_SPRX
	bool use_wm_proxy = file_exists(WM_RES_PATH "/wm_proxy.sprx");
#endif

#ifdef SLAUNCH_FILE
	int fdsl = create_slaunch_file();
#endif

#if defined(PKG_LAUNCHER) || defined(MOUNT_ROMS)
	f1_len = (webman_config->roms ? id_ROMS : webman_config->ps3l ? id_GAMEI : id_VIDEO) + 1;
#endif

	int ns = NONE; u8 uprofile = profile;

#ifdef NET_SUPPORT
	if(g_socket >= 0 && open_remote_dir(g_socket, "/", &abort_connection) < 0) do_umount(false);
#endif

	for(u8 f0 = 0; f0 < 16; f0++)  // drives: 0="/dev_hdd0", 1="/dev_usb000", 2="/dev_usb001", 3="/dev_usb002", 4="/dev_usb003", 5="/dev_usb006", 6="/dev_usb007", 7="/net0", 8="/net1", 9="/net2", 10="/net3", 11="/net4", 12="/ext", 13="/dev_sd", 14="/dev_ms", 15="/dev_cf"
	{
		if(check_drive(f0)) continue;

		if(conn_s_p == START_DAEMON)
		{
			if(webman_config->boots && (f0 >= 1 && f0 <= 6)) // usb000->007
			{
				wait_for(drives[f0], webman_config->boots);
			}
		}

#ifdef USE_NTFS
		if(IS_NTFS && webman_config->ntfs) prepNTFS(0);
#endif
		is_net = IS_NET;

		if(!(is_net || IS_NTFS) && (isDir(drives[f0]) == false)) continue;
//
#ifdef NET_SUPPORT
		if((ns >= 0) && (ns!=g_socket)) sclose(&ns);
#endif
		ns = NONE; uprofile = profile;
		for(u8 f1 = 0; f1 < f1_len; f1++) // paths: 0="GAMES", 1="GAMEZ", 2="PS3ISO", 3="BDISO", 4="DVDISO", 5="PS2ISO", 6="PSXISO", 7="PSXGAMES", 8="PSPISO", 9="ISO", 10="video", 11="GAMEI", 12="ROMS"
		{
#ifndef COBRA_ONLY
			if(IS_ISO_FOLDER && !(IS_PS2_FOLDER)) continue; // 0="GAMES", 1="GAMEZ", 5="PS2ISO", 10="video"
#endif
			if(key >= max_xmb_items) break;

			//if(IS_PS2_FOLDER && f0>0)  continue; // PS2ISO is supported only from /dev_hdd0
#ifdef PKG_LAUNCHER
			if(IS_GAMEI_FOLDER) {if(is_net || (IS_HDD0) || (IS_NTFS)) continue;}
#endif
			if(IS_VIDEO_FOLDER) {if(is_net) continue; else strcpy(paths[id_VIDEO], (IS_HDD0) ? "video" : "GAMES_DUP");}
			if(IS_NTFS)  {if(f1 >= id_ISO) break; else if(IS_JB_FOLDER || (f1 == id_PSXGAMES)) continue;} // 0="GAMES", 1="GAMEZ", 7="PSXGAMES", 9="ISO", 10="video", 11="GAMEI", 12="ROMS"

#ifdef NET_SUPPORT
			if(is_net)
			{
				if(f1 >= id_ISO) break; // ignore 9="ISO", 10="video", 11="GAMEI"
			}
#endif
			if(check_content_type(f1)) continue;

#ifdef NET_SUPPORT
			if(is_net && (netiso_svrid == (f0-7)) && (g_socket != -1)) ns = g_socket; /* reuse current server connection */ else
			if(is_net && (ns<0)) ns = connect_to_remote_server(f0-7);
#endif
			if(is_net && (ns<0)) break;
//
			bool ls; u8 li, subfolder; li=subfolder=0; ls=false; // single letter folder

		subfolder_letter_xml:
			subfolder = 0; uprofile = profile;
		read_folder_xml:
//
#ifdef NET_SUPPORT
			if(is_net)
			{
				char ll[4]; if(li) sprintf(ll, "/%c", '@'+li); else *ll = NULL;
				sprintf(param, "/%s%s%s",    paths[f1], SUFIX(uprofile), ll);

				if(li == 99) sprintf(param, "/%s%s", paths[f1], AUTOPLAY_TAG);
			}
			else
#endif
			{
				if(IS_NTFS)
					sprintf(param, "%s", WMTMP);
				else
				{
					sprintf(param, "%s/%s%s", drives[f0], paths[f1], SUFIX(uprofile));
					if(li == 99) sprintf(param, "%s/%s%s", drives[f0], paths[f1], AUTOPLAY_TAG);
				}
			}

#ifdef NET_SUPPORT
				if(is_net && open_remote_dir(ns, param, &abort_connection) < 0) goto continue_reading_folder_xml; //continue;
#endif
			//led(YELLOW, ON);
			{
				CellFsDirectoryEntry entry; u32 read_e;
				int fd2 = 0, flen, plen;
				char tempID[12];
				u8 is_iso = 0;

#ifdef NET_SUPPORT
				sys_addr_t data2 = NULL;
				int v3_entries, v3_entry; v3_entries=v3_entry=0;
				netiso_read_dir_result_data *data=NULL; char neth[8];
				if(is_net)
				{
					v3_entries = read_remote_dir(ns, &data2, &abort_connection);
					if(!data2) goto continue_reading_folder_xml; //continue;
					data=(netiso_read_dir_result_data*)data2; sprintf(neth, "/net%i", (f0-7));
				}
#endif
				if(!is_net && file_exists(param) == false) goto continue_reading_folder_xml; //continue;
				if(!is_net && cellFsOpendir(param, &fd) != CELL_FS_SUCCEEDED) goto continue_reading_folder_xml; //continue;

				plen = strlen(param);

				while((!is_net && (!cellFsGetDirectoryEntries(fd, &entry, sizeof(entry), &read_e) && read_e > 0))
#ifdef NET_SUPPORT
					|| (is_net && (v3_entry < v3_entries))
#endif
					)
				{
					if(key >= max_xmb_items) break;
#ifdef NET_SUPPORT
					if(is_net)
					{
						if((ls == false) && (li==0) && (f1>1) && (data[v3_entry].is_directory) && (data[v3_entry].name[1]==NULL)) ls=true; // single letter folder was found

						if(add_net_game(ns, data, v3_entry, neth, param, templn, tempstr, enc_dir_name, icon, tempID, f1, 0) == FAILED) {v3_entry++; continue;}
#ifdef SLAUNCH_FILE
						if(key < MAX_SLAUNCH_ITEMS) add_slaunch_entry(fdsl, neth, param, data[v3_entry].name, icon, templn, tempID, f1);
#endif
#ifdef WM_PROXY_SPRX
						if(use_wm_proxy)
							read_e = sprintf(tempstr, "<Table key=\"%04i\">"
											 XML_PAIR("icon","%s")
											 XML_PAIR("title","%s") "%s"
											 XML_PAIR("module_action","/mount_ps3%s%s/%s"),
											 key, icon,
											 templn, XAI_LINK_PAIR, neth, param, enc_dir_name);
						else
#endif
							read_e = sprintf(tempstr, "<Table key=\"%04i\">"
											 XML_PAIR("icon","%s")
											 XML_PAIR("title","%s") "%s"
											 XML_PAIR("module_action","http://%s/mount_ps3%s%s/%s"),
											 key, icon,
											 templn, WEB_LINK_PAIR, local_ip, neth, param, enc_dir_name);

						// info level: 0=Path, 1=Path | titleid, 2=titleid | drive, 3=none
						if(webman_config->info <= 1)
						{
							if((webman_config->info == 1) & HAS_TITLE_ID) {sprintf(folder_name, " | %s" , tempID);} else *folder_name = NULL;
							read_e += sprintf(tempstr + read_e, XML_PAIR("info","%s%s%s"), neth, param, folder_name);
						}
						else if(webman_config->info == 2)
						{
							if(HAS_TITLE_ID)
								read_e += sprintf(tempstr + read_e, XML_PAIR("info","%s | %s"), tempID, drives[f0] + 1);
							else
								read_e += sprintf(tempstr + read_e, XML_PAIR("info","%s"), drives[f0] + 1);

							if(f1 < 2) read_e += sprintf(tempstr + read_e, " | JB");
						}

						sprintf(tempstr + read_e, "</Table>");

						if(add_xmb_entry(f0, f1, plen + 6, tempstr, templn, skey[key].value, key, myxml_ps3, myxml_ps2, myxml_psx, myxml_psp, myxml_dvd, myxml_roms, data[v3_entry].name, item_count, xml_len, 0)) key++;

						v3_entry++;
					}
					else
#endif // #ifdef NET_SUPPORT
					{
						if(entry.entry_name.d_name[0] == '.') continue;

//////////////////////////////
						subfolder = 0;
						sprintf(subpath, "%s/%s", param, entry.entry_name.d_name);
						if(IS_ISO_FOLDER && isDir(subpath) && cellFsOpendir(subpath, &fd2) == CELL_FS_SUCCEEDED)
						{
							strcpy(subpath, entry.entry_name.d_name); subfolder = 1;
next_xml_entry:
							cellFsGetDirectoryEntries(fd2, &entry, sizeof(entry), &read_e);
							if(read_e < 1) {cellFsClosedir(fd2); fd2 = 0; continue;}
							if(entry.entry_name.d_name[0] == '.') goto next_xml_entry;
							sprintf(templn, "%s/%s", subpath, entry.entry_name.d_name); entry.entry_name.d_name[0] = NULL; entry.entry_name.d_namlen = concat(entry.entry_name.d_name, templn);
						}
//////////////////////////////

						if(key >= max_xmb_items) break;

						flen = entry.entry_name.d_namlen; is_iso = is_iso_file(entry.entry_name.d_name, flen, f1, f0);

						if(IS_JB_FOLDER && !is_iso)
						{
#ifdef PKG_LAUNCHER
							if(IS_GAMEI_FOLDER)
							{
								// create game folder in /dev_hdd0/game and copy PARAM.SFO to prevent deletion of XMB icon when gameDATA is disabled
								sprintf(tempstr, "//%s/%s/PARAM.SFO", HDD0_GAME_DIR, entry.entry_name.d_name);
								if(file_exists(tempstr) == false)
								{
									sprintf(templn, "%s/%s/PARAM.SFO", param, entry.entry_name.d_name);
									mkdir_tree(tempstr); file_copy(templn, tempstr, COPY_WHOLE_FILE);
								}

								if(!webman_config->ps3l) continue;

								sprintf(templn, "%s/%s/USRDIR/EBOOT.BIN", param, entry.entry_name.d_name); if(!file_exists(templn)) continue;
								sprintf(templn, "%s/%s/PARAM.SFO", param, entry.entry_name.d_name);
							}
							else
#endif
								sprintf(templn, "%s/%s/PS3_GAME/PARAM.SFO", param, entry.entry_name.d_name);
						}

						if(is_iso || (IS_JB_FOLDER && file_exists(templn)))
						{
							*icon = *tempID = NULL;

							if(!is_iso)
							{
								get_title_and_id_from_sfo(templn, tempID, entry.entry_name.d_name, icon, tempstr, 0);
							}
							else
							{
#ifndef COBRA_ONLY
								get_name(templn, entry.entry_name.d_name, NO_EXT);
#else
								if(get_name_iso_or_sfo(templn, tempID, icon, param, entry.entry_name.d_name, f0, f1, uprofile, flen, tempstr) == FAILED) continue;
#endif
							}

							get_default_icon(icon, param, entry.entry_name.d_name, !is_iso, tempID, ns, abort_connection, f1, f0);
#ifdef SLAUNCH_FILE
							if(key < MAX_SLAUNCH_ITEMS) add_slaunch_entry(fdsl, "", param, entry.entry_name.d_name, icon, templn, tempID, f1);
#endif
							if(webman_config->tid && HAS_TITLE_ID && strlen(templn) < 50 && strstr(templn, " [") == NULL) {sprintf(enc_dir_name, " [%s]", tempID); strcat(templn, enc_dir_name);}

							urlenc(enc_dir_name, entry.entry_name.d_name);

							// subfolder name
							if((IS_NTFS) && entry.entry_name.d_name[0] == '[')
							{
								strcpy(folder_name, entry.entry_name.d_name); *folder_name = '/'; char *p = strstr(folder_name, "] "); if(p) *p = NULL;
							}
							else
							{
								*folder_name = NULL;
								char *p = strchr(entry.entry_name.d_name, '/'); if(p) {*p = NULL; sprintf(folder_name, "/%s", entry.entry_name.d_name); *p = '/';}
							}
#ifdef WM_PROXY_SPRX
							if(use_wm_proxy)
								read_e = sprintf(tempstr, "<Table key=\"%04i\">"
												 XML_PAIR("icon","%s")
												 XML_PAIR("title","%s") "%s"
												 XML_PAIR("module_action","/mount_ps3%s%s/%s"),
												 key, icon,
												 templn, XAI_LINK_PAIR, "", param, enc_dir_name);
							else
#endif
								read_e = sprintf(tempstr, "<Table key=\"%04i\">"
												 XML_PAIR("icon","%s")
												 XML_PAIR("title","%s") "%s"
												 XML_PAIR("module_action","http://%s/mount_ps3%s%s/%s"),
												 key, icon,
												 templn, WEB_LINK_PAIR, local_ip, "", param, enc_dir_name);


							// info level: 0=Path, 1=Path | titleid, 2=titleid | drive, 3=none
							if(webman_config->info <= 1)
							{
								if((webman_config->info == 1) & HAS_TITLE_ID) {strcat(folder_name, " | "); strcat(folder_name, tempID);}
								read_e += sprintf(tempstr + read_e, XML_PAIR("info","%s/%s%s"), drives[f0], paths[f1], folder_name);
							}
							else if(webman_config->info == 2)
							{
								if(HAS_TITLE_ID)
									read_e += sprintf(tempstr + read_e, XML_PAIR("info","%s | %s"), tempID, drives[f0] + 5);
								else
									read_e += sprintf(tempstr + read_e, XML_PAIR("info","%s"), drives[f0] + 5);

								if(f1 < 2) read_e += sprintf(tempstr + read_e, " | JB");
							}

							sprintf(tempstr + read_e, "</Table>");

							if(add_xmb_entry(f0, f1, plen + flen - 13, tempstr, templn, skey[key].value, key, myxml_ps3, myxml_ps2, myxml_psx, myxml_psp, myxml_dvd, myxml_roms, entry.entry_name.d_name, item_count, xml_len, subfolder)) key++;
						}
//////////////////////////////
						if(subfolder) goto next_xml_entry;
//////////////////////////////
					}
				}

				if(!is_net) cellFsClosedir(fd);

#ifdef NET_SUPPORT
				if(data2) {sys_memory_free((sys_addr_t)data2); data2 = NULL;}
#endif
			}
//
continue_reading_folder_xml:

			if((uprofile > 0) && (f1 < id_ISO)) {subfolder = uprofile = 0; goto read_folder_xml;}
			if(!IS_NTFS)
			{
				if(is_net && ls && (li < 27)) {li++; goto subfolder_letter_xml;} else if(li < 99 && f1 < id_ISO) {li = 99; goto subfolder_letter_xml;}
			}
//
		}
#ifdef NET_SUPPORT
		if(is_net && (ns >= 0) && (ns!=g_socket)) sclose(&ns);
#endif
	}

#ifdef SLAUNCH_FILE
	close_slaunch_file(fdsl);
#endif

	if( !(webman_config->nogrp))
	{
#ifndef PKG_LAUNCHER
		if(!(webman_config->cmask & PS3)) {strcat(myxml_ps3, "</Attributes><Items>");}
#else
		if(!(webman_config->cmask & PS3)) {strcat(myxml_ps3, "</Attributes><Items>"); if(webman_config->ps3l && isDir("/dev_hdd0/game/PKGLAUNCH")) strcat(myxml_ps3, QUERY_XMB("pkg_launcher", "xcb://localhost/query?limit=1&cond=Ae+Game:Common.dirPath /dev_hdd0/game+Ae+Game:Common.fileName PKGLAUNCH"));}
#endif
		if(!(webman_config->cmask & PS2)) {strcat(myxml_ps2, "</Attributes><Items>"); if(webman_config->ps2l && isDir(PS2_CLASSIC_PLACEHOLDER)) strcat(myxml_ps2, QUERY_XMB("ps2_classic_launcher", "xcb://127.0.0.1/query?limit=1&cond=Ae+Game:Game.titleId PS2U10000"));}

#ifdef COBRA_ONLY
		if(!(webman_config->cmask & PS1)) {strcat(myxml_psx, "</Attributes><Items>");}
		if(!(webman_config->cmask & PSP)) {strcat(myxml_psp, "</Attributes><Items>"); if(webman_config->pspl && (isDir("/dev_hdd0/game/PSPC66820") || isDir("/dev_hdd0/game/PSPM66820"))) strcat(myxml_psp, QUERY_XMB("cobra_psp_launcher", "xcb://127.0.0.1/query?cond=AGL+Game:Game.titleId PSPC66820 PSPM66820"));}
		if(!(webman_config->cmask & DVD) || !(webman_config->cmask & BLU)) {strcat(myxml_dvd, "</Attributes><Items>"); if(webman_config->rxvid) strcat(myxml_dvd, QUERY_XMB("rx_video", "#seg_wm_bdvd"));}
 #ifdef MOUNT_ROMS
		if(  webman_config->roms       )  {strcat(myxml_roms, "</Attributes><Items>");}
 #endif
#endif
	}
	else
		myxml_dvd[0] = NULL;

	// --- sort scanned content

	led(YELLOW, OFF);
	led(GREEN, ON);

	if(key)
	{   // sort xmb items
		u16 m, n;
		t_keys swap;
		for(n = 0; n < (key - 1); n++)
			for(m = (n + 1); m < key; m++)
				if(strncmp(skey[n].value, skey[m].value, XML_KEY_LEN) > 0)
				{
					swap    = skey[n];
					skey[n] = skey[m];
					skey[m] = swap;
				}
	}

	// --- add eject & setup/xmbm+ menu
#ifdef ENGLISH_ONLY
	bool add_xmbm_plus = file_exists("/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN.xml");
#else
	bool add_xmbm_plus = false;

	while(true)
	{
		sprintf(templn, "/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN%s.xml", lang_code);
		add_xmbm_plus = file_exists(templn);
		if(add_xmbm_plus || *lang_code == NULL) break; *lang_code = NULL;
	}
#endif

	if(webman_config->nogrp)
	{
		if(!add_xmbm_plus) strcat(myxml_items, ADD_XMB_ITEM("eject"));

		if(!webman_config->nosetup)
		{
			if(add_xmbm_plus)
#ifdef ENGLISH_ONLY
				strcat(myxml_items, QUERY_XMB("setup", "xmb://localhost/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN.xml#seg_webman_links_items"));
#else
			{
				sprintf(tempstr, QUERY_XMB("setup", "xmb://localhost%s#seg_webman_links_items"), templn);
				strcat(myxml_items, tempstr);
			}
#endif
			else
				strcat(myxml_items, ADD_XMB_ITEM("setup"));
		}

		xml_len[gPS3] = strlen(myxml_items);
	}
	else
	{
		xml_len[gPS3] = strlen(myxml_ps3);
		xml_len[gPSX] = strlen(myxml_psx);
		xml_len[gPS2] = strlen(myxml_ps2);
		xml_len[gPSP] = strlen(myxml_psp);
		xml_len[gDVD] = strlen(myxml_dvd);
 #ifdef MOUNT_ROMS
		xml_len[gROM] = strlen(myxml_roms);
 #endif
	}

	// --- add sorted items to xml
	if( webman_config->nogrp )
	{
		for(u16 a = 0; a < key; a++)
		{
			if(xml_len[gPS3] >= (BUFFER_SIZE - 1000)) break;
			sprintf(templn, ADD_XMB_ITEM("%s"), skey[a].value + XML_KEY_LEN, skey[a].value + XML_KEY_LEN);
			xml_len[gPS3] += concat(myxml_items + xml_len[gPS3], templn);
		}
	}
	else
	{
		for(u16 a = 0; a < key; a++)
		{
			sprintf(templn, ADD_XMB_ITEM("%s"), skey[a].value + XML_KEY_LEN, skey[a].value + XML_KEY_LEN);
			if(*skey[a].value == PS3_&& xml_len[gPS3] < (BUFFER_SIZE - 5000))
				xml_len[gPS3] += concat(myxml_ps3 + xml_len[gPS3], templn);
			else
			if(*skey[a].value == PS2 && xml_len[gPS2] < (BUFFER_SIZE_PS2 - 128))
				xml_len[gPS2] += concat(myxml_ps2 + xml_len[gPS2], templn);
#ifdef COBRA_ONLY
			else
			if(*skey[a].value == PS1 && xml_len[gPSX] < (BUFFER_SIZE_PSX - 128))
				xml_len[gPSX] += concat(myxml_psx + xml_len[gPSX], templn);
			else
			if(*skey[a].value == PSP && xml_len[gPSP] < (BUFFER_SIZE_PSP - 128))
				xml_len[gPSP] += concat(myxml_psp + xml_len[gPSP], templn);
			else
			if(*skey[a].value == BLU && xml_len[gDVD] < (BUFFER_SIZE_DVD - 1200))
				xml_len[gDVD] += concat(myxml_dvd + xml_len[gDVD], templn);
 #ifdef MOUNT_ROMS
			else
			if(*skey[a].value == DVD && xml_len[gROM] < (BUFFER_SIZE_ROM - 1200))
				xml_len[gROM] += concat(myxml_roms + xml_len[gROM], templn);
 #endif
#endif
		}
	}

	// --- close xml items

	if( !(webman_config->nogrp))
	{
		if(!(webman_config->cmask & PS3)) strcat(myxml_ps3 + xml_len[gPS3], "</Items></View>");
		if(!(webman_config->cmask & PS2)) strcat(myxml_ps2 + xml_len[gPS2], "</Items></View>");
#ifdef COBRA_ONLY
		if(!(webman_config->cmask & PS1)) strcat(myxml_psx + xml_len[gPSX], "</Items></View>");
		if(!(webman_config->cmask & PSP)) strcat(myxml_psp + xml_len[gPSP], "</Items></View>");
		if(!(webman_config->cmask & DVD) || !(webman_config->cmask & BLU))
		{
			strcat(myxml_dvd + xml_len[gDVD], "</Items></View>");
			if(webman_config->rxvid)
			{
				strcat(myxml_dvd + xml_len[gDVD],	"<View id=\"seg_wm_bdvd\">"
													"<Items>"
													QUERY_XMB("rx_video1", "xcb://localhost/query?table=MMS_MEDIA_TYPE_SYSTEM&genre=Video&sort=+StorageMedia:StorageMedia.sortOrder+StorageMedia:StorageMedia.timeInserted&cond=Ae+StorageMedia:StorageMedia.stat.mediaStatus %xCB_MEDIA_INSERTED+Ae+StorageMedia:StorageMedia.mediaFormat %xCB_MEDIA_FORMAT_DATA+AGL+StorageMedia:StorageMedia.type %xCB_MEDIA_TYPE_BDROM %xCB_MEDIA_TYPE_WM")
													QUERY_XMB("rx_video2", "xcb://localhost/query?sort=+Game:Common.titleForSort&cond=AGL+Game:Game.titleId RXMOV0000 RXMOVZZZZ+An+Game:Game.category 2D+An+Game:Game.category BV+An+Game:Game.category HG")
													"</Items>"
													"</View>");
			}
		}
 #ifdef MOUNT_ROMS
		if(webman_config->roms) strcat(myxml_roms + xml_len[gROM], "</Items></View>");
 #endif
#endif
	}

#ifndef ENGLISH_ONLY
	char *buffer = enc_dir_name;
#endif

	// --- build xml headers
	sprintf(myxml,  "%s"
					"<View id=\"seg_mygames\">"
					"<Attributes>", XML_HEADER);

	if(!add_xmbm_plus)
	{
#ifndef ENGLISH_ONLY
		char *STR_EJECTDISC = buffer; //[32];//	= "Eject Disc";
		language("STR_EJECTDISC", STR_EJECTDISC, "Eject Disc");
#endif

#ifdef WM_PROXY_SPRX
		if(use_wm_proxy)
			sprintf(templn, "<Table key=\"eject\">"
							XML_PAIR("icon","%s")
							XML_PAIR("title","%s")
							XML_PAIR("info","%s") "%s"
							XML_PAIR("module_action","/mount_ps3/unmount") "</Table>",
							wm_icons[11], STR_EJECTDISC, STR_UNMOUNTGAME, XAI_LINK_PAIR);
		else
#endif
			sprintf(templn, "<Table key=\"eject\">"
							XML_PAIR("icon","%s")
							XML_PAIR("title","%s")
							XML_PAIR("info","%s") "%s"
							XML_PAIR("module_action","http://%s/mount_ps3/unmount") "</Table>",
							wm_icons[11], STR_EJECTDISC, STR_UNMOUNTGAME, WEB_LINK_PAIR, local_ip);
		strcat(myxml, templn);
	}

	if(!(webman_config->nogrp))
	{
#ifndef ENGLISH_ONLY
		char *STR_PS3FORMAT = buffer; //[40];//	= "PS3 format games";
		language("STR_PS3FORMAT", STR_PS3FORMAT, "PS3 format games");
#endif
		if( !(webman_config->cmask & PS3)) {sprintf(templn, "<Table key=\"wm_ps3\">"
															XML_PAIR("icon","%s")
															XML_PAIR("title","PLAYSTATION\xC2\xAE\x33")
															XML_PAIR("info","%'i %s") "%s",
															wm_icons[gPS3], item_count[gPS3], STR_PS3FORMAT, STR_NOITEM_PAIR); strcat(myxml, templn);}
#ifndef ENGLISH_ONLY
		char *STR_PS2FORMAT = buffer; //[48];//	= "PS2 format games";
		language("STR_PS2FORMAT", STR_PS2FORMAT, "PS2 format games");
#endif
		if( !(webman_config->cmask & PS2)) {sprintf(templn, "<Table key=\"wm_ps2\">"
															XML_PAIR("icon","%s")
															XML_PAIR("title","PLAYSTATION\xC2\xAE\x32")
															XML_PAIR("info","%'i %s") "%s",
															wm_icons[gPS2], item_count[gPS2], STR_PS2FORMAT, STR_NOITEM_PAIR); strcat(myxml, templn);}
#ifdef COBRA_ONLY

#ifndef ENGLISH_ONLY
		char *STR_PS1FORMAT = buffer; //[48];//	= "PSOne format games";
		language("STR_PS1FORMAT", STR_PS1FORMAT, "PSOne format games");
#endif
		if( !(webman_config->cmask & PS1)) {sprintf(templn, "<Table key=\"wm_psx\">"
															XML_PAIR("icon","%s")
															XML_PAIR("title","PLAYSTATION\xC2\xAE")
															XML_PAIR("info","%'i %s") "%s",
															wm_icons[gPSX], item_count[gPSX], STR_PS1FORMAT, STR_NOITEM_PAIR);strcat(myxml, templn);}
#ifndef ENGLISH_ONLY
		char *STR_PSPFORMAT = buffer; //[48];//	= "PSP\xE2\x84\xA2 format games";
		language("STR_PSPFORMAT", STR_PSPFORMAT, "PSP\xE2\x84\xA2 format games");
#endif
		if( !(webman_config->cmask & PSP)) {sprintf(templn, "<Table key=\"wm_psp\">"
															XML_PAIR("icon","%s")
															XML_PAIR("title","PLAYSTATION\xC2\xAEPORTABLE")
															XML_PAIR("info","%'i %s") "%s",
															wm_icons[gPSP], item_count[gPSP], STR_PSPFORMAT, STR_NOITEM_PAIR);strcat(myxml, templn);}
#ifndef ENGLISH_ONLY
		char *STR_VIDFORMAT = buffer; //[56];//	= "Blu-ray\xE2\x84\xA2 and DVD";
		char *STR_VIDEO = buffer + 100; //[40];//		= "Video content";

		language("STR_VIDFORMAT", STR_VIDFORMAT, "Blu-ray\xE2\x84\xA2 and DVD");
		language("STR_VIDEO", STR_VIDEO, "Video content");
#endif
		if( !(webman_config->cmask & DVD) ||
			!(webman_config->cmask & BLU)) {sprintf(templn, "<Table key=\"wm_dvd\">"
															XML_PAIR("icon","%s")
															XML_PAIR("title","%s")
															XML_PAIR("info","%'i %s") "%s",
															wm_icons[gDVD], STR_VIDFORMAT, item_count[gDVD], STR_VIDEO, STR_NOITEM_PAIR); strcat(myxml, templn);}
 #ifdef MOUNT_ROMS
		if(webman_config->roms && (item_count[gROM] > 0))
		{
											sprintf(templn, "<Table key=\"wm_rom\">"
															XML_PAIR("icon%s", "%s")
															XML_PAIR("title","ROMS")
															XML_PAIR("info","%'i %s") "%s",
															covers_exist[7] ? "" : "_rsc",
															covers_exist[7] ? WM_ICONS_PATH "/icon_wm_album_emu.png" : "item_tex_ps3util",
															item_count[gROM], "ROMS", STR_NOITEM_PAIR);strcat(myxml, templn);
		}
 #endif
#endif
	}

	if(!webman_config->nosetup)
	{
#ifndef ENGLISH_ONLY
		char *STR_WMSETUP2 = buffer; //[56];//	= "Setup webMAN options";
		language("STR_WMSETUP2", STR_WMSETUP2, "Setup webMAN options");
#endif
		sprintf(templn, "<Table key=\"setup\">"
						 XML_PAIR("icon","%s")
						 XML_PAIR("title","%s")
						 XML_PAIR("info","%s") "%s",
						 add_xmbm_plus ? "/dev_hdd0/game/XMBMANPLS/USRDIR/IMAGES/multiman.png" : wm_icons[10],
						 STR_WMSETUP, STR_WMSETUP2, WEB_LINK_PAIR); strcat(myxml, templn);

		if(add_xmbm_plus)
			strcat(myxml, XML_PAIR("child","segment"));
		else
			{sprintf(templn, XML_PAIR("module_action","http://%s/setup.ps3"), local_ip); strcat(myxml, templn);}

		strcat(myxml, "</Table>");
	}

#ifndef ENGLISH_ONLY
	close_language();
#endif

	if(!(webman_config->nogrp))
	{
		strcat(myxml, "</Attributes><Items>");
		if( !(webman_config->nosetup) )
		{
			if(add_xmbm_plus)
#ifdef ENGLISH_ONLY
				strcat(myxml, QUERY_XMB("setup", "xmb://localhost/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN.xml#seg_webman_links_items"));
#else
			{
				sprintf(templn, QUERY_XMB("setup", "xmb://localhost/dev_hdd0/game/XMBMANPLS/USRDIR/FEATURES/webMAN%s.xml#seg_webman_links_items"), lang_code);
				strcat(myxml, templn);
			}
#endif
			else
				strcat(myxml, ADD_XMB_ITEM("setup"));
		}

		if(!add_xmbm_plus) strcat(myxml, ADD_XMB_ITEM("eject"));

		if(!(webman_config->cmask & PS3)) strcat(myxml, QUERY_XMB("wm_ps3", "#seg_wm_ps3_items"));
		if(!(webman_config->cmask & PS2)) strcat(myxml, QUERY_XMB("wm_ps2", "#seg_wm_ps2_items"));
#ifdef COBRA_ONLY
		if(!(webman_config->cmask & PS1)) strcat(myxml, QUERY_XMB("wm_psx", "#seg_wm_psx_items"));
		if(!(webman_config->cmask & PSP)) strcat(myxml, QUERY_XMB("wm_psp", "#seg_wm_psp_items"));
		if(!(webman_config->cmask & DVD) ||
		   !(webman_config->cmask & BLU)) strcat(myxml, QUERY_XMB("wm_dvd", "#seg_wm_dvd_items"));
 #ifdef MOUNT_ROMS
		if(  webman_config->roms        ) strcat(myxml, QUERY_XMB("wm_rom", "#seg_wm_rom_items"));
 #endif
#endif

		strcat(myxml, "</Items></View>");
	}

	// --- save xml file
	int fdxml = 0, slen;
	if(cellFsOpen(MY_GAMES_XML, CELL_FS_O_CREAT | CELL_FS_O_TRUNC | CELL_FS_O_WRONLY, &fdxml, NULL, 0) == CELL_FS_SUCCEEDED)
	{
		cellFsWrite(fdxml, (char*)myxml, strlen(myxml), NULL);

		if( (webman_config->nogrp))
		{
			cellFsWrite(fdxml, (char*)myxml_ps3, strlen(myxml_ps3), NULL);
			cellFsWrite(fdxml, (char*)"</Attributes><Items>", 20, NULL);
			cellFsWrite(fdxml, (char*)myxml_items, strlen(myxml_items), NULL);
			slen = sprintf(myxml, "%s%s", "</Items></View>", "</XMBML>\r\n");
		}
		else
		{
			if(!(webman_config->cmask & PS3)) cellFsWrite(fdxml, (char*)myxml_ps3, strlen(myxml_ps3), NULL);
			if(!(webman_config->cmask & PS2)) cellFsWrite(fdxml, (char*)myxml_ps2, strlen(myxml_ps2), NULL);
#ifdef COBRA_ONLY
			if(!(webman_config->cmask & PS1)) cellFsWrite(fdxml, (char*)myxml_psx, strlen(myxml_psx), NULL);
			if(!(webman_config->cmask & PSP)) cellFsWrite(fdxml, (char*)myxml_psp, strlen(myxml_psp), NULL);
			if(!(webman_config->cmask & DVD) || !(webman_config->cmask & BLU)) cellFsWrite(fdxml, (char*)myxml_dvd, strlen(myxml_dvd), NULL);
 #ifdef MOUNT_ROMS
			if(webman_config->roms) cellFsWrite(fdxml, (char*)myxml_roms, strlen(myxml_roms), NULL);
 #endif
#endif
			slen = sprintf(myxml, "</XMBML>\r\n");
		}

		cellFsWrite(fdxml, (char*)myxml, slen, NULL);
		cellFsClose(fdxml);

		cellFsChmod(MY_GAMES_XML, MODE);
	}

#ifdef LAUNCHPAD
	bool launchpad_xml = !(webman_config->launchpad_xml) && file_exists(LAUNCHPAD_FILE_XML);

	if(launchpad_xml)
	{
		*myxml_ps3 = *param = *tempstr = *templn = NULL;
		game_listing(myxml_ps3, templn, param, tempstr, LAUNCHPAD_MODE, false);
	}
#endif

	led(GREEN, ON);

	// --- release allocated memory
#ifdef USE_VM
	sys_vm_unmap(sysmem);
#else
	sys_memory_free(sysmem);
#endif

	return false;
}

static void update_xml_thread(u64 conn_s_p)
{
	refreshing_xml = 1;

	if(IS_ON_XMB)
		while(View_Find("explore_plugin") == 0) sys_ppu_thread_sleep(1); // wait for explore_plugin

	if(scan_mygames_xml(conn_s_p)) mount_autoboot();

	refreshing_xml = 0;
	sys_ppu_thread_exit(0);
}
