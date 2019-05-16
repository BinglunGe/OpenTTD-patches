/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tbtr_template_vehicle.h Template-based train replacement: template vehicle header. */

#ifndef TEMPLATE_VEH_H
#define TEMPLATE_VEH_H

#include "company_func.h"

#include "vehicle_type.h"
#include "vehicle_base.h"
#include "vehicle_func.h"

#include "articulated_vehicles.h"
#include "newgrf_callbacks.h"
#include "newgrf_engine.h"
#include "newgrf_spritegroup.h"

#include "engine_base.h"
#include "engine_type.h"
#include "engine_func.h"

#include "sortlist_type.h"

#include "zoom_func.h"

#define FOR_ALL_TEMPLATES_FROM(var, start) FOR_ALL_ITEMS_FROM(TemplateVehicle, template_index, var, start)
#define FOR_ALL_TEMPLATES(var) FOR_ALL_TEMPLATES_FROM(var, 0)

#define FOR_ALL_TEMPLATE_REPLACEMENTS_FROM(var, start) FOR_ALL_ITEMS_FROM(TemplateReplacement, template_replacement_index, var, start)
#define FOR_ALL_TEMPLATE_REPLACEMENTS(var) FOR_ALL_TEMPLATE_REPLACEMENTS_FROM(var, 0)

struct TemplateVehicle;
struct TemplateReplacement;

typedef uint16 TemplateID;


static const uint16 CONSIST_HEAD = 0x0;
static const uint16 CONSIST_TAIL = 0xffff;

/** A pool allowing to store up to ~64k templates */
typedef Pool<TemplateVehicle, TemplateID, 512, 0x10000> TemplatePool;
extern TemplatePool _template_pool;

/// listing/sorting templates
typedef GUIList<const TemplateVehicle*> GUITemplateList;

struct TemplateVehicleImageDimensions {
	int reference_width;
	int vehicle_pitch;
	int cached_veh_length;

	void SetFromTrain(const Train *t);

	int GetDisplayImageWidth() const
	{
		return ScaleGUITrad(this->cached_veh_length * this->reference_width / VEHICLE_LENGTH);
	}

	int GetOffsetX() const
	{
		return ScaleGUITrad(this->reference_width) / 2;
	}

	int GetOffsetY() const
	{
		return ScaleGUITrad(this->vehicle_pitch);
	}
};

struct TemplateVehicle : TemplatePool::PoolItem<&_template_pool>, BaseVehicle {
private:
	TemplateVehicle *next;                      ///< pointer to the next vehicle in the chain
	TemplateVehicle *previous;                  ///< NOSAVE: pointer to the previous vehicle in the chain
	TemplateVehicle *first;                     ///< NOSAVE: pointer to the first vehicle in the chain

public:
	friend const SaveLoad* GTD();
	friend void AfterLoadTemplateVehicles();

	// Template usage configuration
	bool reuse_depot_vehicles;
	bool keep_remaining_vehicles;
	bool refit_as_template;
	bool replace_old_only;

	// Things derived from a virtual train
	TemplateVehicle *other_multiheaded_part; ///< Multiheaded Engine support
	Money value;                        ///< Value of the vehicle
	OwnerByte owner;

	EngineID engine_type;               ///< The type of engine used for this vehicle.
	CargoID cargo_type;                 ///< type of cargo this vehicle is carrying
	uint16 cargo_cap;                   ///< total capacity
	byte cargo_subtype;

	byte subtype;
	RailTypeByte railtype;

	VehicleID index;

	uint16 real_consist_length;

	uint16 max_speed;
	uint32 power;
	uint32 weight;
	uint32 max_te;

	VehicleSpriteSeq sprite_seq;                     ///< NOSAVE: Vehicle appearance.
	TemplateVehicleImageDimensions image_dimensions; ///< NOSAVE: image dimensions

	TemplateVehicle(VehicleType type = VEH_INVALID, EngineID e = INVALID_ENGINE, byte B = 0, Owner = _local_company);
	TemplateVehicle(EngineID, RailVehicleInfo*);

	TemplateVehicle(EngineID eid)
	{
		next = NULL;
		previous = NULL;
		first = this;
		engine_type = eid;
		this->reuse_depot_vehicles = true;
		this->keep_remaining_vehicles = true;
		this->refit_as_template = true;
		this->replace_old_only = false;
		this->sprite_seq.count = 1;
	}

	~TemplateVehicle();

	inline TemplateVehicle* Next() const { return this->next; }
	inline TemplateVehicle* Prev() const { return this->previous; }
	inline TemplateVehicle* First() const { return this->first; }

	void SetNext(TemplateVehicle*);
	void SetPrev(TemplateVehicle*);
	void SetFirst(TemplateVehicle*);

	TemplateVehicle* GetNextUnit() const;
	TemplateVehicle* GetPrevUnit();

	bool IsSetReuseDepotVehicles() const { return this->reuse_depot_vehicles; }
	bool IsSetKeepRemainingVehicles() const { return this->keep_remaining_vehicles; }
	bool IsSetRefitAsTemplate() const { return this->refit_as_template; }
	bool IsReplaceOldOnly() const { return this->replace_old_only; }
	void ToggleReuseDepotVehicles() { this->reuse_depot_vehicles = !this->reuse_depot_vehicles; }
	void ToggleKeepRemainingVehicles() { this->keep_remaining_vehicles = !this->keep_remaining_vehicles; }
	void ToggleRefitAsTemplate() { this->refit_as_template = !this->refit_as_template; }
	void ToggleReplaceOldOnly() { this->replace_old_only = !this->replace_old_only; }

	bool IsPrimaryVehicle() const { return this->IsFrontEngine(); }
	inline bool IsFrontEngine() const { return HasBit(this->subtype, GVSF_FRONT); }
	inline bool HasArticulatedPart() const { return this->Next() != NULL && this->Next()->IsArticulatedPart(); }

	inline bool IsArticulatedPart() const { return HasBit(this->subtype, GVSF_ARTICULATED_PART); }
	inline bool IsMultiheaded() const { return HasBit(this->subtype, GVSF_MULTIHEADED); }

	inline bool IsFreeWagonChain() const { return HasBit(this->subtype, GVSF_FREE_WAGON); }

	// since CmdBuildTemplateVehicle(...)
	inline void SetFrontEngine()     { SetBit(this->subtype, GVSF_FRONT); }
	inline void SetEngine()          { SetBit(this->subtype, GVSF_ENGINE); }
	inline void SetArticulatedPart() { SetBit(this->subtype, GVSF_ARTICULATED_PART); }
	inline void SetMultiheaded()     { SetBit(this->subtype, GVSF_MULTIHEADED); }

	inline void SetWagon() { SetBit(this->subtype, GVSF_WAGON); }
	inline void SetFreeWagon() { SetBit(this->subtype, GVSF_FREE_WAGON); }

	inline uint16 GetRealLength() const { return this->real_consist_length; }
	inline void SetRealLength(uint16 len) { this->real_consist_length = len; }

	int Length() const;

	SpriteID GetImage(Direction) const;
	SpriteID GetSpriteID() const;

	short NumGroupsUsingTemplate() const;

};

// TemplateReplacement stuff

typedef Pool<TemplateReplacement, uint16, 16, 1024> TemplateReplacementPool;
extern TemplateReplacementPool _template_replacement_pool;

struct TemplateReplacement : TemplateReplacementPool::PoolItem<&_template_replacement_pool> {
	GroupID group;
	TemplateID sel_template;

	TemplateReplacement(GroupID gid, TemplateID tid) { this->group=gid; this->sel_template=tid; }
	TemplateReplacement() {}
	~TemplateReplacement() {}

	inline GroupID Group() { return this->group; }
	inline GroupID Template() { return this->sel_template; }

	inline void SetGroup(GroupID gid) { this->group = gid; }
	inline void SetTemplate(TemplateID tid) { this->sel_template = tid; }

	inline TemplateID GetTemplateVehicleID() { return sel_template; }
};

TemplateReplacement* GetTemplateReplacementByGroupID(GroupID);
bool IssueTemplateReplacement(GroupID, TemplateID);

short DeleteTemplateReplacementsByGroupID(GroupID);

#endif /* TEMPLATE_VEH_H */
