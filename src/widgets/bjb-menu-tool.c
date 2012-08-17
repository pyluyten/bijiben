#include "bjb-menu-tool.h"

struct _BijibenMenuButtonPrivate {
	GtkMenu* menu;
	gboolean popup_in_progress;
};

static gpointer bijiben_menu_button_parent_class = NULL;

BijibenMenuButton* bijiben_menu_button_construct (GType object_type, GtkWidget *image)
{
	BijibenMenuButton * self = NULL;
	GtkArrow* arrow;
	GtkGrid* grid;
    
	self = (BijibenMenuButton*) g_object_new (object_type, NULL);
	gtk_button_set_focus_on_click ((GtkButton*) self, FALSE);
	arrow = g_object_ref_sink ((GtkArrow*) gtk_arrow_new (GTK_ARROW_DOWN, 
                                                        GTK_SHADOW_NONE));

  grid = g_object_ref_sink((GtkGrid*)gtk_grid_new());
	gtk_orientable_set_orientation ((GtkOrientable*) grid, 
                                  GTK_ORIENTATION_HORIZONTAL);
    
	gtk_container_add((GtkContainer*) grid, (GtkWidget*) image);
	gtk_container_add ((GtkContainer*) grid, (GtkWidget*) arrow);
	gtk_grid_set_column_spacing (grid, (guint) 3);
	gtk_container_add ((GtkContainer*) self, (GtkWidget*) grid);
	gtk_button_set_relief(GTK_BUTTON(self),GTK_RELIEF_NONE);
	_g_object_unref0 (grid);
	_g_object_unref0 (arrow);
	return self;
}

BijibenMenuButton* bijiben_menu_button_new (GtkWidget *image) 
{
	return bijiben_menu_button_construct (BIJIBEN_TYPE_MENU_BUTTON, image);
}


static gpointer _g_object_ref0 (gpointer self) {
	return self ? g_object_ref (self) : NULL;
}


static void bijiben_menu_button_menu_position (BijibenMenuButton* self, GtkMenu* menu, gint* x, gint* y, gboolean* push_in) {
	gint _vala_x = 0;
	gint _vala_y = 0;
	gboolean _vala_push_in = FALSE;
	GtkAllocation allocation = {0};
	GtkAllocation _tmp0_ = {0};
	gint sx;
	gint sy;
	gboolean _tmp1_ = FALSE;
	GdkWindow* _tmp8_ = NULL;
	gint _tmp9_;
	gint _tmp10_;
	gint _tmp11_ = 0;
	gint _tmp12_ = 0;
	GtkRequisition menu_req = {0};
	GdkRectangle monitor = {0};
	GtkMenu* _tmp13_;
	GtkRequisition _tmp14_ = {0};
	GtkTextDirection _tmp15_ = 0;
	gint _tmp22_;
	GdkWindow* _tmp23_ = NULL;
	GdkWindow* _tmp24_;
	GdkWindow* window;
	GdkScreen* _tmp25_ = NULL;
	GdkScreen* _tmp26_;
	GdkScreen* screen;
	GdkScreen* _tmp27_;
	GdkWindow* _tmp28_;
	gint _tmp29_ = 0;
	gint monitor_num;
	gint _tmp30_;
	GdkScreen* _tmp31_;
	gint _tmp32_;
	GdkRectangle _tmp33_ = {0};
	gint _tmp34_;
	GdkRectangle _tmp35_;
	gint _tmp36_;
	GdkRectangle _tmp52_;
	gint _tmp53_;
	GdkRectangle _tmp54_;
	gint _tmp55_;
	gint _tmp56_;
	GtkAllocation _tmp57_;
	gint _tmp58_;
	GtkRequisition _tmp59_;
	gint _tmp60_;
	GtkMenu* _tmp88_;
	gint _tmp89_;
	GtkMenu* _tmp90_;
	GtkWidget* _tmp91_ = NULL;
	GtkWindow* _tmp92_;
	GtkWindow* toplevel;
	gboolean _tmp93_ = FALSE;
	GtkWindow* _tmp94_;
	gboolean _tmp97_;
	g_return_if_fail (self != NULL);
	g_return_if_fail (menu != NULL);
	gtk_widget_get_allocation ((GtkWidget*) self, &_tmp0_);
	allocation = _tmp0_;
	sx = 0;
	sy = 0;
	_tmp1_ = gtk_widget_get_has_window ((GtkWidget*) self);
	if (!_tmp1_) {
		gint _tmp2_;
		GtkAllocation _tmp3_;
		gint _tmp4_;
		gint _tmp5_;
		GtkAllocation _tmp6_;
		gint _tmp7_;
		_tmp2_ = sx;
		_tmp3_ = allocation;
		_tmp4_ = _tmp3_.x;
		sx = _tmp2_ + _tmp4_;
		_tmp5_ = sy;
		_tmp6_ = allocation;
		_tmp7_ = _tmp6_.y;
		sy = _tmp5_ + _tmp7_;
	}
	_tmp8_ = gtk_widget_get_window ((GtkWidget*) self);
	_tmp9_ = sx;
	_tmp10_ = sy;
	gdk_window_get_root_coords (_tmp8_, _tmp9_, _tmp10_, &_tmp11_, &_tmp12_);
	sx = _tmp11_;
	sy = _tmp12_;
	_tmp13_ = menu;
	gtk_widget_get_preferred_size ((GtkWidget*) _tmp13_, NULL, &_tmp14_);
	menu_req = _tmp14_;
	_tmp15_ = gtk_widget_get_direction ((GtkWidget*) self);
	if (_tmp15_ == GTK_TEXT_DIR_LTR) {
		gint _tmp16_;
		_tmp16_ = sx;
		_vala_x = _tmp16_;
	} else {
		gint _tmp17_;
		GtkAllocation _tmp18_;
		gint _tmp19_;
		GtkRequisition _tmp20_;
		gint _tmp21_;
		_tmp17_ = sx;
		_tmp18_ = allocation;
		_tmp19_ = _tmp18_.width;
		_tmp20_ = menu_req;
		_tmp21_ = _tmp20_.width;
		_vala_x = (_tmp17_ + _tmp19_) - _tmp21_;
	}
	_tmp22_ = sy;
	_vala_y = _tmp22_;
	_tmp23_ = gtk_widget_get_window ((GtkWidget*) self);
	_tmp24_ = _g_object_ref0 (_tmp23_);
	window = _tmp24_;
	_tmp25_ = gtk_widget_get_screen ((GtkWidget*) self);
	_tmp26_ = _g_object_ref0 (_tmp25_);
	screen = _tmp26_;
	_tmp27_ = screen;
	_tmp28_ = window;
	_tmp29_ = gdk_screen_get_monitor_at_window (_tmp27_, _tmp28_);
	monitor_num = _tmp29_;
	_tmp30_ = monitor_num;
	if (_tmp30_ < 0) {
		monitor_num = 0;
	}
	_tmp31_ = screen;
	_tmp32_ = monitor_num;
	gdk_screen_get_monitor_geometry (_tmp31_, _tmp32_, &_tmp33_);
	monitor = _tmp33_;
	_tmp34_ = _vala_x;
	_tmp35_ = monitor;
	_tmp36_ = _tmp35_.x;
	if (_tmp34_ < _tmp36_) {
		GdkRectangle _tmp37_;
		gint _tmp38_;
		_tmp37_ = monitor;
		_tmp38_ = _tmp37_.x;
		_vala_x = _tmp38_;
	} else {
		gint _tmp39_;
		GtkRequisition _tmp40_;
		gint _tmp41_;
		GdkRectangle _tmp42_;
		gint _tmp43_;
		GdkRectangle _tmp44_;
		gint _tmp45_;
		_tmp39_ = _vala_x;
		_tmp40_ = menu_req;
		_tmp41_ = _tmp40_.width;
		_tmp42_ = monitor;
		_tmp43_ = _tmp42_.x;
		_tmp44_ = monitor;
		_tmp45_ = _tmp44_.width;
		if ((_tmp39_ + _tmp41_) > (_tmp43_ + _tmp45_)) {
			GdkRectangle _tmp46_;
			gint _tmp47_;
			GdkRectangle _tmp48_;
			gint _tmp49_;
			GtkRequisition _tmp50_;
			gint _tmp51_;
			_tmp46_ = monitor;
			_tmp47_ = _tmp46_.x;
			_tmp48_ = monitor;
			_tmp49_ = _tmp48_.width;
			_tmp50_ = menu_req;
			_tmp51_ = _tmp50_.width;
			_vala_x = (_tmp47_ + _tmp49_) - _tmp51_;
		}
	}
	_tmp52_ = monitor;
	_tmp53_ = _tmp52_.y;
	_tmp54_ = monitor;
	_tmp55_ = _tmp54_.height;
	_tmp56_ = _vala_y;
	_tmp57_ = allocation;
	_tmp58_ = _tmp57_.height;
	_tmp59_ = menu_req;
	_tmp60_ = _tmp59_.height;
	if ((((_tmp53_ + _tmp55_) - _tmp56_) - _tmp58_) >= _tmp60_) {
		gint _tmp61_;
		GtkAllocation _tmp62_;
		gint _tmp63_;
		_tmp61_ = _vala_y;
		_tmp62_ = allocation;
		_tmp63_ = _tmp62_.height;
		_vala_y = _tmp61_ + _tmp63_;
	} else {
		gint _tmp64_;
		GdkRectangle _tmp65_;
		gint _tmp66_;
		GtkRequisition _tmp67_;
		gint _tmp68_;
		_tmp64_ = _vala_y;
		_tmp65_ = monitor;
		_tmp66_ = _tmp65_.y;
		_tmp67_ = menu_req;
		_tmp68_ = _tmp67_.height;
		if ((_tmp64_ - _tmp66_) >= _tmp68_) {
			gint _tmp69_;
			GtkRequisition _tmp70_;
			gint _tmp71_;
			_tmp69_ = _vala_y;
			_tmp70_ = menu_req;
			_tmp71_ = _tmp70_.height;
			_vala_y = _tmp69_ - _tmp71_;
		} else {
			GdkRectangle _tmp72_;
			gint _tmp73_;
			GdkRectangle _tmp74_;
			gint _tmp75_;
			gint _tmp76_;
			GtkAllocation _tmp77_;
			gint _tmp78_;
			gint _tmp79_;
			GdkRectangle _tmp80_;
			gint _tmp81_;
			_tmp72_ = monitor;
			_tmp73_ = _tmp72_.y;
			_tmp74_ = monitor;
			_tmp75_ = _tmp74_.height;
			_tmp76_ = _vala_y;
			_tmp77_ = allocation;
			_tmp78_ = _tmp77_.height;
			_tmp79_ = _vala_y;
			_tmp80_ = monitor;
			_tmp81_ = _tmp80_.y;
			if ((((_tmp73_ + _tmp75_) - _tmp76_) - _tmp78_) > (_tmp79_ - _tmp81_)) {
				gint _tmp82_;
				GtkAllocation _tmp83_;
				gint _tmp84_;
				_tmp82_ = _vala_y;
				_tmp83_ = allocation;
				_tmp84_ = _tmp83_.height;
				_vala_y = _tmp82_ + _tmp84_;
			} else {
				gint _tmp85_;
				GtkRequisition _tmp86_;
				gint _tmp87_;
				_tmp85_ = _vala_y;
				_tmp86_ = menu_req;
				_tmp87_ = _tmp86_.height;
				_vala_y = _tmp85_ - _tmp87_;
			}
		}
	}
	_tmp88_ = menu;
	_tmp89_ = monitor_num;
	gtk_menu_set_monitor (_tmp88_, _tmp89_);
	_tmp90_ = menu;
	_tmp91_ = gtk_widget_get_parent ((GtkWidget*) _tmp90_);
	_tmp92_ = _g_object_ref0 (GTK_IS_WINDOW (_tmp91_) ? ((GtkWindow*) _tmp91_) : NULL);
	toplevel = _tmp92_;
	_tmp94_ = toplevel;
	if (_tmp94_ != NULL) {
		GtkWindow* _tmp95_;
		gboolean _tmp96_ = FALSE;
		_tmp95_ = toplevel;
		_tmp96_ = gtk_widget_get_visible ((GtkWidget*) _tmp95_);
		_tmp93_ = !_tmp96_;
	} else {
		_tmp93_ = FALSE;
	}
	_tmp97_ = _tmp93_;
	if (_tmp97_) {
		GtkWindow* _tmp98_;
		_tmp98_ = toplevel;
		gtk_window_set_type_hint (_tmp98_, GDK_WINDOW_TYPE_HINT_DROPDOWN_MENU);
	}
	_vala_push_in = FALSE;
	_g_object_unref0 (toplevel);
	_g_object_unref0 (screen);
	_g_object_unref0 (window);
	if (x) {
		*x = _vala_x;
	}
	if (y) {
		*y = _vala_y;
	}
	if (push_in) {
		*push_in = _vala_push_in;
	}
}


static void _bijiben_menu_button_menu_position_gtk_menu_position_func (GtkMenu* menu, gint* x, gint* y, gboolean* push_in, gpointer self) {
	bijiben_menu_button_menu_position (self, menu, x, y, push_in);
}


static void bijiben_menu_button_real_toggled (GtkToggleButton* base) {
	BijibenMenuButton * self;
	GtkStyleContext* _tmp0_ = NULL;
	GtkStyleContext* _tmp1_;
	GtkStyleContext* context;
	gboolean _tmp2_ = FALSE;
	self = (BijibenMenuButton*) base;
	_tmp0_ = gtk_widget_get_style_context ((GtkWidget*) self);
	_tmp1_ = _g_object_ref0 (_tmp0_);
	context = _tmp1_;
	_tmp2_ = gtk_toggle_button_get_active ((GtkToggleButton*) self);
	if (_tmp2_) {
		gboolean _tmp3_;
		GtkStyleContext* _tmp6_;
		GtkStyleContext* _tmp7_;
		_tmp3_ = self->priv->popup_in_progress;
		if (!_tmp3_) {
			GtkMenu* _tmp4_;
			guint32 _tmp5_ = 0U;
			g_signal_emit_by_name (self, "popup");
			_tmp4_ = self->priv->menu;
			_tmp5_ = gtk_get_current_event_time ();
			gtk_menu_popup (_tmp4_, NULL, NULL, _bijiben_menu_button_menu_position_gtk_menu_position_func, self, (guint) 1, _tmp5_);
		}
		_tmp6_ = context;
		gtk_style_context_add_class (_tmp6_, GTK_STYLE_CLASS_MENUBAR);
		_tmp7_ = context;
		gtk_style_context_add_class (_tmp7_, GTK_STYLE_CLASS_MENUITEM);
	} else {
		GtkStyleContext* _tmp8_;
		GtkStyleContext* _tmp9_;
		GtkMenu* _tmp10_;
		_tmp8_ = context;
		gtk_style_context_remove_class (_tmp8_, GTK_STYLE_CLASS_MENUBAR);
		_tmp9_ = context;
		gtk_style_context_remove_class (_tmp9_, GTK_STYLE_CLASS_MENUITEM);
		_tmp10_ = self->priv->menu;
		gtk_menu_popdown (_tmp10_);
	}
	gtk_widget_reset_style ((GtkWidget*) self);
	_g_object_unref0 (context);
}


static gboolean bijiben_menu_button_real_button_press_event (GtkWidget* base, GdkEventButton* event) {
	BijibenMenuButton * self;
	gboolean result = FALSE;
	GtkWidget* _tmp0_ = NULL;
	GtkWidget* _tmp1_;
	GtkWidget* ewidget;
	gboolean _tmp2_ = FALSE;
	GtkWidget* _tmp3_;
	gboolean _tmp5_;
	GtkMenu* _tmp6_;
	guint32 _tmp7_ = 0U;
	self = (BijibenMenuButton*) base;
	g_return_val_if_fail (event != NULL, FALSE);
	_tmp0_ = gtk_get_event_widget ((GdkEvent*) (event));
	_tmp1_ = _g_object_ref0 (_tmp0_);
	ewidget = _tmp1_;
	_tmp3_ = ewidget;
	if (_tmp3_ != GTK_WIDGET (self)) {
		_tmp2_ = TRUE;
	} else {
		gboolean _tmp4_ = FALSE;
		_tmp4_ = gtk_toggle_button_get_active ((GtkToggleButton*) self);
		_tmp2_ = _tmp4_;
	}
	_tmp5_ = _tmp2_;
	if (_tmp5_) {
		result = FALSE;
		_g_object_unref0 (ewidget);
		return result;
	}
	g_signal_emit_by_name (self, "popup");
	_tmp6_ = self->priv->menu;
	_tmp7_ = gtk_get_current_event_time ();
	gtk_menu_popup (_tmp6_, NULL, NULL, _bijiben_menu_button_menu_position_gtk_menu_position_func, self, (guint) 1, _tmp7_);
	gtk_toggle_button_set_active ((GtkToggleButton*) self, TRUE);
	self->priv->popup_in_progress = TRUE;
	result = TRUE;
	_g_object_unref0 (ewidget);
	return result;
}


static gboolean bijiben_menu_button_real_button_release_event (GtkWidget* base, GdkEventButton* event) {
	BijibenMenuButton * self;
	gboolean result = FALSE;
	gboolean _tmp0_;
	gboolean popup_in_progress_saved;
	GtkWidget* _tmp1_ = NULL;
	GtkWidget* _tmp2_;
	GtkWidget* ewidget;
	gboolean _tmp3_ = FALSE;
	gboolean _tmp4_ = FALSE;
	GtkWidget* _tmp5_;
	gboolean _tmp7_;
	gboolean _tmp9_;
	GtkWidget* _tmp11_;
	self = (BijibenMenuButton*) base;
	g_return_val_if_fail (event != NULL, FALSE);
	_tmp0_ = self->priv->popup_in_progress;
	popup_in_progress_saved = _tmp0_;
	self->priv->popup_in_progress = FALSE;
	_tmp1_ = gtk_get_event_widget ((GdkEvent*) (event));
	_tmp2_ = _g_object_ref0 (_tmp1_);
	ewidget = _tmp2_;
	_tmp5_ = ewidget;
	if (_tmp5_ == GTK_WIDGET (self)) {
		gboolean _tmp6_;
		_tmp6_ = popup_in_progress_saved;
		_tmp4_ = !_tmp6_;
	} else {
		_tmp4_ = FALSE;
	}
	_tmp7_ = _tmp4_;
	if (_tmp7_) {
		gboolean _tmp8_ = FALSE;
		_tmp8_ = gtk_toggle_button_get_active ((GtkToggleButton*) self);
		_tmp3_ = _tmp8_;
	} else {
		_tmp3_ = FALSE;
	}
	_tmp9_ = _tmp3_;
	if (_tmp9_) {
		GtkMenu* _tmp10_;
		_tmp10_ = self->priv->menu;
		gtk_menu_popdown (_tmp10_);
		result = TRUE;
		_g_object_unref0 (ewidget);
		return result;
	}
	_tmp11_ = ewidget;
	if (_tmp11_ != GTK_WIDGET (self)) {
		GtkMenu* _tmp12_;
		_tmp12_ = self->priv->menu;
		gtk_menu_popdown (_tmp12_);
		result = TRUE;
		_g_object_unref0 (ewidget);
		return result;
	}
	result = FALSE;
	_g_object_unref0 (ewidget);
	return result;
}


static void bijiben_menu_button_menu_show (BijibenMenuButton* self, GtkWidget* menu) {
	g_return_if_fail (self != NULL);
	g_return_if_fail (menu != NULL);
	self->priv->popup_in_progress = TRUE;
	gtk_toggle_button_set_active ((GtkToggleButton*) self, TRUE);
	self->priv->popup_in_progress = FALSE;
}


static void bijiben_menu_button_menu_hide (BijibenMenuButton* self, GtkWidget* menu) {
	g_return_if_fail (self != NULL);
	g_return_if_fail (menu != NULL);
	gtk_toggle_button_set_active ((GtkToggleButton*) self, FALSE);
}


static void bijiben_menu_button_menu_detach (BijibenMenuButton* self, GtkMenu* menu) {
	g_return_if_fail (self != NULL);
	g_return_if_fail (menu != NULL);
}


static void _bijiben_menu_button_menu_show_gtk_widget_show (GtkWidget* _sender, gpointer self) {
	bijiben_menu_button_menu_show (self, _sender);
}


static void _bijiben_menu_button_menu_hide_gtk_widget_hide (GtkWidget* _sender, gpointer self) {
	bijiben_menu_button_menu_hide (self, _sender);
}


static void _bijiben_menu_button_menu_detach_gtk_menu_detach_func (GtkWidget* attach_widget, GtkMenu* menu) {
	bijiben_menu_button_menu_detach ((BijibenMenuButton*)attach_widget, menu);
}


void bijiben_menu_button_set_menu (BijibenMenuButton* self, GtkMenu* menu) {
	GtkMenu* _tmp6_;
	GtkMenu* _tmp7_;
	GtkMenu* _tmp8_;
	g_return_if_fail (self != NULL);

  if (self->priv->menu != NULL) {
		GtkMenu* _tmp1_;
		guint _tmp2_ = 0U;
		GtkMenu* _tmp3_;
		guint _tmp4_ = 0U;
		GtkMenu* _tmp5_;
		_tmp1_ = self->priv->menu;
		g_signal_parse_name ("show", GTK_TYPE_WIDGET, &_tmp2_, NULL, FALSE);
		g_signal_handlers_disconnect_matched ((GtkWidget*) _tmp1_, G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA, _tmp2_, 0, NULL, (GCallback) _bijiben_menu_button_menu_show_gtk_widget_show, self);
		_tmp3_ = self->priv->menu;
		g_signal_parse_name ("hide", GTK_TYPE_WIDGET, &_tmp4_, NULL, FALSE);
		g_signal_handlers_disconnect_matched ((GtkWidget*) _tmp3_, G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA, _tmp4_, 0, NULL, (GCallback) _bijiben_menu_button_menu_hide_gtk_widget_hide, self);
		_tmp5_ = self->priv->menu;
		gtk_menu_detach (_tmp5_);
	}
	_tmp6_ = menu;
	_tmp7_ = _g_object_ref0 (_tmp6_);
	_g_object_unref0 (self->priv->menu);
	self->priv->menu = _tmp7_;
	_tmp8_ = self->priv->menu;
	if (_tmp8_ != NULL) {
		GtkMenu* _tmp9_;
		GtkMenu* _tmp10_;
		GtkMenu* _tmp11_;
		_tmp9_ = self->priv->menu;
		g_signal_connect_object ((GtkWidget*) _tmp9_, "show", (GCallback) _bijiben_menu_button_menu_show_gtk_widget_show, self, 0);
		_tmp10_ = self->priv->menu;
		g_signal_connect_object ((GtkWidget*) _tmp10_, "hide", (GCallback) _bijiben_menu_button_menu_hide_gtk_widget_hide, self, 0);
		_tmp11_ = self->priv->menu;
		gtk_menu_attach_to_widget (_tmp11_, (GtkWidget*) self, _bijiben_menu_button_menu_detach_gtk_menu_detach_func);
	}
}


static gboolean bijiben_menu_button_real_draw (GtkWidget* base, cairo_t* cr) {
	BijibenMenuButton * self;
	gboolean result = FALSE;
	cairo_t* _tmp0_;
	self = (BijibenMenuButton*) base;
	g_return_val_if_fail (cr != NULL, FALSE);
	_tmp0_ = cr;
	GTK_WIDGET_CLASS (bijiben_menu_button_parent_class)->draw ((GtkWidget*) GTK_TOGGLE_BUTTON (self), _tmp0_);
	result = FALSE;
	return result;
}


static void bijiben_menu_button_finalize (GObject* obj) {
	BijibenMenuButton * self;
	self = BIJIBEN_MENU_BUTTON (obj);
	bijiben_menu_button_set_menu (self, NULL);
	_g_object_unref0 (self->priv->menu);
	G_OBJECT_CLASS (bijiben_menu_button_parent_class)->finalize (obj);
}

static void bijiben_menu_button_class_init (BijibenMenuButtonClass * klass) {
	bijiben_menu_button_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (BijibenMenuButtonPrivate));
	GTK_TOGGLE_BUTTON_CLASS (klass)->toggled = bijiben_menu_button_real_toggled;
	GTK_WIDGET_CLASS (klass)->button_press_event = bijiben_menu_button_real_button_press_event;
	GTK_WIDGET_CLASS (klass)->button_release_event = bijiben_menu_button_real_button_release_event;
	GTK_WIDGET_CLASS (klass)->draw = bijiben_menu_button_real_draw;
	G_OBJECT_CLASS (klass)->finalize = bijiben_menu_button_finalize;
	g_signal_new ("popup", BIJIBEN_TYPE_MENU_BUTTON, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void bijiben_menu_button_instance_init (BijibenMenuButton * self) {
	self->priv = BIJIBEN_MENU_BUTTON_GET_PRIVATE (self);
}


GType bijiben_menu_button_get_type (void) {
	static volatile gsize bijiben_menu_button_type_id__volatile = 0;
	if (g_once_init_enter (&bijiben_menu_button_type_id__volatile)) {
		static const GTypeInfo g_define_type_info = { sizeof (BijibenMenuButtonClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) bijiben_menu_button_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (BijibenMenuButton), 0, (GInstanceInitFunc) bijiben_menu_button_instance_init, NULL };
		GType bijiben_menu_button_type_id;
		bijiben_menu_button_type_id = g_type_register_static (GTK_TYPE_TOGGLE_BUTTON, "BijibenMenuButton", &g_define_type_info, 0);
		g_once_init_leave (&bijiben_menu_button_type_id__volatile, bijiben_menu_button_type_id);
	}
	return bijiben_menu_button_type_id__volatile;
}
