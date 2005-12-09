/*
 * $Id$
 * 
 * XaAES - XaAES Ain't the AES (c) 1992 - 1998 C.Graham
 *                                 1999 - 2003 H.Robbers
 *                                        2004 F.Naumann & O.Skancke
 *
 * A multitasking AES replacement for FreeMiNT
 *
 * This file is part of XaAES.
 *
 * XaAES is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * XaAES is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XaAES; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include RSCHNAME

#include "xa_global.h"
#include "xa_pdlg.h"
#include "xa_wdlg.h"

#include "k_main.h"
#include "k_mouse.h"

#include "app_man.h"
#include "form.h"
#include "obtree.h"
#include "draw_obj.h"
#include "c_window.h"
#include "rectlist.h"
#include "widgets.h"
#include "xa_graf.h"
#include "xa_rsrc.h"
#include "xa_form.h"
#include "scrlobjc.h"

#include "xa_user_things.h"
#include "nkcc.h"
#include "mint/signal.h"


/*
 * WDIALOG FUNCTIONS (pdlg)
 *
 * documentation about this can be found in:
 *
 * - the GEMLIB documentation:
 *   http://arnaud.bercegeay.free.fr/gemlib/
 *
 * - the MagiC documentation project:
 *   http://www.bygjohn.fsnet.co.uk/atari/mdp/
 */

#if WDIALOG_PDLG

extern struct toolbar_handlers wdlg_th;

static DRV_INFO *
xv_create_driver_info(XVDIPB *vpb, short handle, short id)
{
	DRV_INFO *d = NULL;

	vpb->intin[0] = id;
	vpb->control[V_N_PTSOUT] = 0;
	vpb->control[V_N_INTOUT] = 0;
	VDI(vpb, 180, 0, 1, 0, handle);
	if (vpb->control[V_N_INTOUT] >= 2)
		d = ptr_from_shorts(vpb->intout[0], vpb->intout[1]); //(*(DRV_INFO **)&vpb->intout[0]);
	return d;
}
	
static short
xv_delete_driver_info(XVDIPB *vpb, short handle, DRV_INFO *d)
{
	ptr_to_shorts(d, pb->intin);	//*(DRV_INFO**)&vpb->intin[0] = d;
	VDI(vpb, 181, 0, 2, 0, handle);
	return vpb->intout[0];
}

static short
xv_read_default_settings(XVDIPB *vpb, short handle, PRN_SETTINGS *p)
{
	ptr_to_shorts(p, vpb->intin); //*(PRN_SETTINGS **)&vpb->intin[0] = p;
	vpb->control[V_N_PTSOUT] = 0;
	vpb->control[V_N_INTOUT] = 0;
	VDI(vpb, 182, 0, 2, 0, handle);
	if (vpb->control[V_N_INTOUT] >= 1)
		return vpb->intout[0];
	return 0;
}

static short
xv_write_default_settings(XVDIPB *vpb, short handle, PRN_SETTINGS *p)
{
	ptr_to_shorts(p, vpb->intin); //*(PRN_SETTINGS **)&vpb->intin[0] = p;
	vpb->control[V_N_PTSOUT] = 0;
	vpb->control[V_N_INTOUT] = 0;
	VDI(vpb, 182, 0, 2, 1, handle);
	if (vpb->control[V_N_INTOUT] >= 1)
		return vpb->intout[0];
	return 0;
}

static short
xvq_devinfo(XVDIPB *vpb, short handle, short dev_id)
{
	vpb->intin[0] = dev_id;
	VDI(vpb, 248, 0, 1, 0, handle);
	return vpb->intout[0];
}

static short
xvq_ext_devinfo(XVDIPB *vpb, short handle, short id, char *path, char *fname, char *dname)
{
	vpb->intin[0] = id;
	ptr_to_shorts(path, vpb->intin  + 1);
	ptr_to_shorts(fname, vpb->intin + 3);
	ptr_to_shorts(dname, vpb->intin + 5);
	
// 	*(char **)&vpb->intin[1] = path;
// 	*(char **)&vpb->intin[3] = fname;
// 	*(char **)&vpb->intin[5] = dname;
	VDI(vpb, 248, 0, 7, 4242, handle);
	return vpb->intout[0];
}

static struct xa_window *
get_pdlg_wind(struct xa_client *client, void *pdlg_ptr)
{
	struct xa_pdlg_info *pdlg = lookup_xa_data(&client->xa_data, pdlg_ptr);
	
	if (pdlg)
		return pdlg->wind;
	else
		return NULL;
}

static void _cdecl
delete_pdlg_info(void *_pdlg)
{
	struct xa_pdlg_info *pdlg = _pdlg;

	if (pdlg)
	{
		if (pdlg->dwt)
		{
			pdlg->dwt->links--;
			remove_wt(pdlg->dwt, false);
			pdlg->dwt = NULL;
		}
		if (pdlg->mwt)
		{
			pdlg->mwt->links--;
			remove_wt(pdlg->mwt, false);
			pdlg->mwt = NULL;
		}
		kfree(pdlg);
	}
}

static int
click_list(SCROLL_INFO *list, SCROLL_ENTRY *this, const struct moose_data *md)
{
	return 0;
}

static void
read_prn_settings(struct xa_pdlg_info *pdlg, PRN_SETTINGS *s)
{
	
}

static struct xa_pdlg_info *
create_new_pdlg(struct xa_client *client, struct xa_window *wind)
{
	struct xa_pdlg_info *pdlg;

	pdlg = kmalloc(sizeof(*pdlg));
	if (pdlg)
	{
		OBJECT *mtree = NULL, *dtree = NULL;
		struct widget_tree *mwt = NULL, *dwt = NULL;
		struct scroll_info *list = NULL;

		bzero(pdlg, sizeof(*pdlg));
		
		mtree = duplicate_obtree(C.Aes, ResourceTree(C.Aes_rsc, WDLG_PDLG), 0);
		
		if (mtree)
		{
			if ((mwt = new_widget_tree(client, mtree)))
			{
				mwt->flags |= WTF_AUTOFREE | WTF_TREE_ALLOC;
				obj_init_focus(mwt, OB_IF_RESET);
				mwt->links++;
			}
			else goto fail;
		}
		
		dtree = duplicate_obtree(C.Aes, ResourceTree(C.Aes_rsc, PDLG_DIALOGS), 0);
		if (dtree)
		{
			if ((dwt = new_widget_tree(client, dtree)))
			{
				dwt->flags |= WTF_AUTOFREE | WTF_TREE_ALLOC;
				dwt->links++;
			}
			else goto fail;
		}

		if (mwt && dwt)
		{
			pdlg->handle = (void *)((unsigned long)pdlg >> 16 | (unsigned long)pdlg << 16);
			pdlg->wind = wind;
			pdlg->dwt = dwt;
			pdlg->mwt = mwt;

			list = set_slist_object( 0,
						 mwt,
						 wind,
						 XPDLG_LIST,
						 SIF_SELECTABLE|SIF_AUTOSELECT,
						 NULL, NULL,			/* scrl_widget closer, fuller*/
						 NULL, click_list, NULL, NULL,	/* scrl_click dclick, click, click_nesticon, keybd */
						 NULL, NULL, NULL, NULL,
						 NULL, NULL, pdlg, 1);
			if (!list)
				goto fail;
			
			pdlg->list = list;

			read_prn_settings(pdlg, &pdlg->current_settings);
			
			add_xa_data(&client->xa_data, pdlg, 0, NULL, delete_pdlg_info);
		}
		else
		{
fail:
			if (mwt)
			{
				mwt->links--;
				remove_wt(mwt, false);
			}
			else if (mtree)
				free_object_tree(client, mtree);
			
			if (dwt)
			{
				dwt->links--;
				remove_wt(dwt, false);
			}
			else if (dtree)
				free_object_tree(client, dtree);		

			if (pdlg)
			{
				kfree(pdlg);
				pdlg = NULL;
			}
		}
	}
	return pdlg;
}
unsigned long
XA_pdlg_create(enum locks lock, struct xa_client *client, AESPB *pb)
{
	struct xa_pdlg_info *pdlg = NULL;
	struct xa_window *wind = NULL;
	OBJECT *obtree = NULL;
	XA_TREE *wt = NULL;
	XA_WIND_ATTR tp = MOVER|NAME;
	RECT r, or;

	DIAG((D_pdlg, client, "XA_pdlg_create"));
	display("XA_pbdl_create: %s", client->name);
	pb->addrout[0] = 0L;

	obtree = ResourceTree(C.Aes_rsc, WDLG_PDLG);
	
	ob_rectangle(obtree, 0, &or);
	
	r = calc_window(lock, client, WC_BORDER,
			tp, created_for_WDIAL,
			client->options.thinframe,
			client->options.thinwork,
			*(RECT *)&or);

	if (!(wind = create_window(lock,
			     send_app_message,
			     NULL,
			     client,
			     false,
			     tp,
			     created_for_WDIAL,
			     client->options.thinframe,
			     client->options.thinwork,
			     r, NULL, NULL)))
		goto memerr;

	if (!(pdlg = create_new_pdlg(client, wind)))
		goto memerr;

	set_toolbar_widget(lock, wind, client, pdlg->mwt->tree, -2, 0, true, &wdlg_th, &or);

	pb->addrout[0] = (long)pdlg->handle;

	display(" -- exit");
	return XAC_DONE;

memerr:
	if (wt)
		remove_wt(wt, true);
	else if (obtree)
		free_object_tree(C.Aes, obtree);

	if (wind)
		delete_window(lock, wind);
	
	if (pdlg)
		kfree(pdlg);
	
	display(" -- exit error");
	return XAC_DONE;

}

unsigned long
XA_pdlg_delete(enum locks lock, struct xa_client *client, AESPB *pb)
{
	struct xa_pdlg_info *pdlg;
	struct xa_window *wind;

	DIAG((D_pdlg, client, "XA_pdlg_delete"));
	display("XA_pdlg_delete: %s", client->name);
	
	pb->intout[0] = 0;

	pdlg = (struct xa_pdlg_info *)((unsigned long)pb->addrin[0] >> 16 | (unsigned long)pb->addrin[0] << 16);
	if (pdlg && (wind = get_pdlg_wind(client, pdlg)))
	{
		if (wind->window_status & XAWS_OPEN)
			close_window(lock, wind);

		delayed_delete_window(lock, wind);
		delete_xa_data(&client->xa_data, pdlg);

		pb->intout[0] = 1;
	}
	display(" -- exit %d", pb->intout[0]);
	return XAC_DONE;
}

static void
get_document_name(struct xa_pdlg_info *pdlg, const char *name)
{
	strncpy(pdlg->document_name, name, 255);
}

unsigned long
XA_pdlg_open(enum locks lock, struct xa_client *client, AESPB *pb)
{
	struct xa_pdlg_info *pdlg;
	struct xa_window *wind;

	DIAG((D_pdlg, client, "XA_pdlg_open"));
	display("XA_pdlg_open: %s", client->name);
	
	pb->intout[0] = 0;

	pdlg = (struct xa_pdlg_info *)((unsigned long)pb->addrin[0] >> 16 | (unsigned long)pb->addrin[0] << 16);
	if (pdlg && (wind = get_pdlg_wind(client, pdlg)))
	{
		if (!(wind->window_status & XAWS_OPEN))
		{
			struct xa_widget *widg = get_widget(wind, XAW_TOOLBAR);
			struct widget_tree *wt = pdlg->mwt;
			RECT r = wind->wa;
			XA_WIND_ATTR tp = wind->active_widgets | MOVER|NAME;
		
			widg->m.properties |= WIP_NOTEXT;
			set_toolbar_handlers(&wdlg_th, wind, widg, widg->stuff);

			obj_init_focus(wt, OB_IF_RESET);

			if (pb->intin[1] == -1 || pb->intin[2] == -1)
			{
				r.x = (root_window->wa.w - r.w) / 2;
				r.y = (root_window->wa.h - r.h) / 2;
			}
			else
			{
				r.x = pb->intin[1];
				r.y = pb->intin[2];
			}
			{
				RECT or;

				obj_area(wt, 0, &or);
				or.x = r.x;
				or.y = r.y;
				change_window_attribs(lock, client, wind, tp, true, or, NULL);
			}

			pdlg->option_flags = pb->intin[0];
			pdlg->settings = (PRN_SETTINGS *)pb->addrin[1];
			get_document_name(pdlg, (const char *)pb->addrin[2]);
			
			wt->tree->ob_x = wind->wa.x;
			wt->tree->ob_y = wind->wa.y;
			if (!wt->zen)
			{
				wt->tree->ob_x += wt->ox;
				wt->tree->ob_y += wt->oy;
			}
			open_window(lock, wind, wind->rc);
		}
		pb->intout[0] = wind->handle;
	}
	display(" -- exit %d", pb->intout[0]);
	return XAC_DONE;
}

unsigned long
XA_pdlg_close(enum locks lock, struct xa_client *client, AESPB *pb)
{
	struct xa_pdlg_info *pdlg;
	struct xa_window *wind;

	DIAG((D_pdlg, client, "XA_pdlg_close"));
	display("XA_pdlg_close: %s", client->name);
	pb->intout[0] = 0;

	pdlg = (struct xa_pdlg_info *)((unsigned long)pb->addrin[0] >> 16 | (unsigned long)pb->addrin[0] << 16);
	if (pdlg && (wind = get_pdlg_wind(client, pdlg)))
	{
		pb->intout[0] = 1;
		pb->intout[1] = pdlg->mwt->tree->ob_x;
		pb->intout[2] = pdlg->mwt->tree->ob_y;
		close_window(lock, wind);			
	}
	display(" -- exit %d", pb->intout[0]);
	return XAC_DONE;
}

unsigned long
XA_pdlg_get(enum locks lock, struct xa_client *client, AESPB *pb)
{
	long *o = (long *)&pb->intout[0];
	
	DIAG((D_pdlg, client, "XA_pdlg_get"));
	
	display("XA_pdlg_get: %d %s", pb->intin[0], client->name);

	pb->intout[0] = 1;

	switch (pb->intin[0])
	{
		case 0:		/* pdlg_get_setsize */
		{
			*o = sizeof(PRN_SETTINGS);
			break;
		}
		default:
		{
			*o = 0L;
		}
	}
	display(" -- exit %x/%x", pb->intout[0], pb->intout[1]);	
	return XAC_DONE;
}

/*
 * Called by delete_xa_data()
 */
static void
delete_usr_settings(void *_us)
{
	struct xa_usr_prn_settings *us = _us;

	if (us->settings)
	{
		display("freeing usr_settings %lx", us->settings);
		ufree(us->settings);
	}
	display("freeing usr_prn_settings %lx", us);
	kfree(us);
}

unsigned long
XA_pdlg_set(enum locks lock, struct xa_client *client, AESPB *pb)
{
	struct xa_pdlg_info *pdlg;
	struct xa_window *wind;
	short ret = -1;

	DIAG((D_pdlg, client, "XA_pdlg_set"));

	display("XA_pdlg_set: %d %s", pb->intin[0], client->name);
	
	pdlg = (struct xa_pdlg_info *)((unsigned long)pb->addrin[0] >> 16 | (unsigned long)pb->addrin[0] << 16);
	if (pdlg && (wind = get_pdlg_wind(client, pdlg)))
	{
		switch (pb->intin[0])
		{
			case 0:		/* pdlg_add_printers	*/
			{
				
				break;
			}
			case 1:		/* pdlg_remove_printers	*/
			{
				break;
			}
			case 2:		/* Update		*/
			{
				break;
			}
			case 3:		/* pdlg_add_dialogs	*/
			{
				break;
			}
			case 4:		/* pdlg_remove_dialogs	*/
			{
				break;
			}
			case 5:		/* pdlg_new_settings	*/
			{
				struct xa_usr_prn_settings *us;
				PRN_SETTINGS *new;
				
				us = kmalloc(sizeof(*us));
				new = umalloc(sizeof(*new));
				if (us && new)
				{
					us->flags = 1;
					us->settings = new;
					memcpy(new, &pdlg->current_settings, sizeof(PRN_SETTINGS));
					add_xa_data(&client->xa_data, us, (long)new, "pdlg_usersett", delete_usr_settings);
				}
				else if (us)
				{
					kfree(us);
					us = NULL;
				}
				else if (new)
				{
					ufree(new);
					new = NULL;
				}
				pb->addrout[0] = (long)new;
				break;
			}
			case 6:		/* pdlg_free_settings	*/
			{
				struct xa_usr_prn_settings *us;
				PRN_SETTINGS *s = (PRN_SETTINGS *)pb->addrin[0];
				
				us = lookup_xa_data_byid(&client->xa_data, (long)s);
				
				ret = 0;
				if (us)
				{
					delete_xa_data(&client->xa_data, us);
					ret = 1;
				}
				
				break;
			}
			case 7:		/* pdlg_dflt_settings	*/
			{
				memcpy((char *)pb->addrin[1], &pdlg->current_settings, sizeof(PRN_SETTINGS));
				break;
			}
			case 8:		/* pdlg_validate_settings */
			{
				break;
			}
			case 9:		/* pdlg_use_settings	*/
			{
				break;
			}
			default:
			{
				break;
			}
		}
	}
	
	if (ret != -1)
		pb->intout[0] = ret;

	display(" -- exit %d", pb->intout[0]);
	return XAC_DONE;
}

static int
check_internal_objects(struct xa_pdlg_info *pdlg, short obj)
{
	OBJECT *obtree = pdlg->mwt->tree;
	int ret = 0;

	if (obtree)
		obtree = NULL;

	/* return 1 if internal button, 0 otherwise */
	return ret;
}

unsigned long
XA_pdlg_evnt(enum locks lock, struct xa_client *client, AESPB *pb)
{
	struct xa_pdlg_info *pdlg;
	struct xa_window *wind;
	short ret = 0;

	DIAG((D_pdlg, client, "XA_pdlg_evnt"));
	display("XA_pdlg_evnt: %s", client->name);

	pdlg = (struct xa_pdlg_info *)((unsigned long)pb->addrin[0] >> 16 | (unsigned long)pb->addrin[0] << 16);
	if (pdlg && (wind = get_pdlg_wind(client, pdlg)))
	{
		OBJECT *obtree = pdlg->mwt->tree;
		struct wdlg_evnt_parms wep;

		wep.wind	= wind;
		wep.wt		= get_widget(wind, XAW_TOOLBAR)->stuff;
		wep.ev		= (EVNT *)pb->addrin[2];
		wep.wdlg	= NULL;
		wep.callout	= NULL;
		wep.redraw	= wdialog_redraw;
		wep.obj		= 0;

		ret = wdialog_event(lock, client, &wep);
		
		if (check_internal_objects(pdlg, wep.obj))
		{
			wep.obj = 0;
			ret = 1;
		}
		else
		{
			if (wep.obj > 0 && (obtree[wep.obj].ob_state & OS_SELECTED))
			{
				obj_change(pdlg->mwt, wind->vdi_settings, wep.obj, -1, obtree[wep.obj].ob_state & ~OS_SELECTED, obtree[wep.obj].ob_flags, true, &wind->wa, wind->rect_list.start, 0);
				ret = 0;
				wep.obj = 0;
			}
			/* prepare return stuff here */
		}
		pb->intout[1] = wep.obj;
	}
	pb->intout[0] = ret;

	DIAG((D_pdlg, client, " --- return %d, obj=%d",
		pb->intout[0], pb->intout[1]));
	
	display(" -- exit %d obj %d", pb->intout[0], pb->intout[1]);

	return XAC_DONE;
}
static FormKeyInput	Keypress;
static FormExit		Formexit;

static bool
Keypress(enum locks lock,
	 struct xa_client *client,
	 struct xa_window *wind,
	 struct widget_tree *wt,
	 const struct rawkey *key,
	 struct fmd_result *ret_fr)
{
	bool no_exit;

	if ((no_exit = Key_form_do(lock, client, wind, wt, key, NULL)))
	{
		struct scroll_info *list;
		struct xa_pdlg_info *pdlg;
		
		wt = get_widget(wind, XAW_TOOLBAR)->stuff;
		list = object_get_slist(wt->tree + XPDLG_LIST);
		pdlg = list->data;
	}
	return no_exit;	
}

static void
Formexit( struct xa_client *client,
	  struct xa_window *wind,
	  XA_TREE *wt,
	  struct fmd_result *fr)
{
	struct xa_pdlg_info *pdlg = (struct xa_pdlg_info *)(object_get_slist(wt->tree + XPDLG_LIST))->data;
	
	if (!check_internal_objects(pdlg, fr->obj))
	{
		pdlg->exit_button = fr->obj >= 0 ? fr->obj : 0;
		client->usr_evnt = 1;
	}
}

static struct toolbar_handlers pdlg_th =
{
	Formexit,		/* FormExit		*exitform;	*/
	Keypress,		/* FormKeyInput		*keypress;	*/

	NULL,			/* DisplayWidget	*display;	*/
	NULL,			/* WidgetBehaviour	*click;		*/
	NULL,			/* WidgetBehaviour	*drag;		*/
	NULL,			/* WidgetBehaviour	*release;	*/
	NULL,			/* void (*destruct)(struct xa_widget *w); */
};

unsigned long
XA_pdlg_do(enum locks lock, struct xa_client *client, AESPB *pb)
{
	struct xa_pdlg_info *pdlg;
	struct xa_window *wind;

	DIAG((D_pdlg, client, "XA_pdlg_do"));

	pb->intout[0] = 0;
	
	pdlg = (struct xa_pdlg_info *)((unsigned long)pb->addrin[0] >> 16 | (unsigned long)pb->addrin[0] << 16);
	if (pdlg && (wind = get_pdlg_wind(client, pdlg)))
	{
		XA_TREE *wt = pdlg->mwt;
		OBJECT *obtree = wt->tree;
		RECT or;
		XA_WIND_ATTR tp = wind->active_widgets & ~STD_WIDGETS;

		ob_rectangle(obtree, 0, &or);
		center_rect(&or);

		change_window_attribs(lock, client, wind, tp, true, or, NULL);
		
		set_toolbar_handlers(&pdlg_th, wind, get_widget(wind, XAW_TOOLBAR), get_widget(wind, XAW_TOOLBAR)->stuff);
		wt->flags |= WTF_FBDO_SLIST;
	/* ............. */
		open_window(lock, wind, wind->rc);

		client->status |= CS_FORM_DO;
		Block(client, 0);
		client->status &= ~CS_FORM_DO;
		
		wt->flags &= ~WTF_FBDO_SLIST;

		close_window(lock, wind);
		
		pb->intout[0] = pdlg->exit_button;
	
	}
	return XAC_DONE;
}

#endif
