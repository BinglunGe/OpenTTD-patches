/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file linkgraph_gui.h Declaration of linkgraph overlay GUI. */

#ifndef LINKGRAPH_GUI_H
#define LINKGRAPH_GUI_H

#include "../company_func.h"
#include "../station_base.h"
#include "../widget_type.h"
#include "../window_gui.h"
#include "linkgraph_base.h"
#include <map>
#include <vector>

/**
 * Monthly statistics for a link between two stations.
 * Only the cargo type of the most saturated linkgraph is taken into account.
 */
struct LinkProperties {
	LinkProperties() : capacity(0), usage(0), planned(0), cargo(CT_INVALID), time(0), shared(false) {}

	/** Return the usage of the link to display. */
	uint Usage() const { return std::max(this->usage, this->planned); }

	uint capacity; ///< Capacity of the link.
	uint usage;    ///< Actual usage of the link.
	uint planned;  ///< Planned usage of the link.
	CargoID cargo; ///< Cargo type of the link.
	uint32 time;   ///< Travel time of the link.
	bool shared;   ///< If this is a shared link to be drawn dashed.
};

/**
 * Handles drawing of links into some window.
 * The window must either be a smallmap or have a valid viewport.
 */
class LinkGraphOverlay {
public:
	struct StationSupplyInfo {
		StationID id;
		uint quantity;
		Point pt;
	};

	struct LinkInfo {
		StationID from_id;
		StationID to_id;
		Point from_pt;
		Point to_pt;
		LinkProperties prop;
	};

	typedef std::vector<StationSupplyInfo> StationSupplyList;
	typedef std::vector<LinkInfo> LinkList;

	static const uint8 LINK_COLOURS[][12];

	/**
	 * Create a link graph overlay for the specified window.
	 * @param w Window to be drawn into.
	 * @param wid ID of the widget to draw into.
	 * @param cargo_mask Bitmask of cargoes to be shown.
	 * @param company_mask Bitmask of companies to be shown.
	 * @param scale Desired thickness of lines and size of station dots.
	 */
	LinkGraphOverlay(Window *w, uint wid, CargoTypes cargo_mask, uint32 company_mask, uint scale) :
			window(w), widget_id(wid), cargo_mask(cargo_mask), company_mask(company_mask), scale(scale), dirty(true)
	{}

	void RebuildCache(bool incremental = false);
	bool CacheStillValid() const;
	void MarkStationViewportLinksDirty(const Station *st);
	void PrepareDraw();
	void Draw(const DrawPixelInfo *dpi) const;
	void SetCargoMask(CargoTypes cargo_mask);
	void SetCompanyMask(uint32 company_mask);

	bool ShowTooltip(Point pt, TooltipCloseCondition close_cond);

	/** Mark the linkgraph dirty to be rebuilt next time Draw() is called. */
	void SetDirty() { this->dirty = true; }

	/** Get a bitmask of the currently shown cargoes. */
	CargoTypes GetCargoMask() { return this->cargo_mask; }

	/** Get a bitmask of the currently shown companies. */
	uint32 GetCompanyMask() { return this->company_mask; }

protected:
	Window *window;                    ///< Window to be drawn into.
	const uint widget_id;              ///< ID of Widget in Window to be drawn to.
	CargoTypes cargo_mask;             ///< Bitmask of cargos to be displayed.
	uint32 company_mask;               ///< Bitmask of companies to be displayed.
	LinkList cached_links;             ///< Cache for links to reduce recalculation.
	StationSupplyList cached_stations; ///< Cache for stations to be drawn.
	Rect cached_region;                ///< Region covered by cached_links and cached_stations.
	uint scale;                        ///< Width of link lines.
	bool dirty;                        ///< Set if overlay should be rebuilt.
	uint64 last_update_number = 0;     ///< Last window update number

	Point GetStationMiddle(const Station *st) const;

	void RefreshDrawCache();
	void DrawLinks(const DrawPixelInfo *dpi) const;
	void DrawStationDots(const DrawPixelInfo *dpi) const;
	void DrawContent(const DrawPixelInfo *dpi, Point pta, Point ptb, const LinkProperties &cargo) const;
	bool IsLinkVisible(Point pta, Point ptb, const DrawPixelInfo *dpi, int padding = 0) const;
	bool IsPointVisible(Point pt, const DrawPixelInfo *dpi, int padding = 0) const;
	void GetWidgetDpi(DrawPixelInfo *dpi, uint margin = 0) const;

	static void AddStats(CargoID new_cargo, uint new_cap, uint new_usg, uint new_plan, uint32 time, bool new_shared, LinkProperties &cargo);
	static void DrawVertex(const DrawPixelInfo *dpi, int x, int y, int size, int colour, int border_colour);
};

void ShowLinkGraphLegend();

/**
 * Menu window to select cargoes and companies to show in a link graph overlay.
 */
struct LinkGraphLegendWindow : Window {
public:
	LinkGraphLegendWindow(WindowDesc *desc, int window_number);
	void SetOverlay(LinkGraphOverlay *overlay);

	void UpdateWidgetSize(int widget, Dimension *size, const Dimension &padding, Dimension *fill, Dimension *resize) override;
	void DrawWidget(const Rect &r, int widget) const override;
	bool OnTooltip(Point pt, int widget, TooltipCloseCondition close_cond) override;
	void OnClick(Point pt, int widget, int click_count) override;
	void OnInvalidateData(int data = 0, bool gui_scope = true) override;

private:
	LinkGraphOverlay *overlay;

	void UpdateOverlayCompanies();
	void UpdateOverlayCargoes();
};

#endif /* LINKGRAPH_GUI_H */
