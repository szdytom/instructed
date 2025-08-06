#ifndef ISTD_CORE_ITEM_H
#define ISTD_CORE_ITEM_H

#include <cstdint>

namespace istd {

consteval std::uint32_t id_string(const char *str) {
	std::uint32_t id = 0;
	for (int i = 0; i < 4; ++i) {
		if (str[i] == '\0') {
			break;
		}
		id |= static_cast<std::uint32_t>(str[i]) << (i * 8);
	}
	return id;
}

enum class ItemType : std::uint32_t {
	Null = 0,

	// Materials
	// Raw
	Sand = id_string("sand"),
	Rock = id_string("rock"),
	Ice = id_string("ice"),
	Salt = id_string("salt"),
	Algae = id_string("alga"),
	Coal = id_string("coal"),
	Hematite = id_string("Fe+O"),
	Titanomagnetite = id_string("TiFe"),
	Gibbsite = id_string("Al+O"),

	// Fundamentals
	Concrete = id_string("ccrt"),
	Plastic = id_string("pltc"),
	Ferrum = id_string("Fe"),
	Titanium = id_string("Ti"),
	Aluminium = id_string("Al"),
	Silicon = id_string("Si"),
	Glass = id_string("glas"),
	Lithium = id_string("Li"),
	Ammonia = id_string("NH3"),
	Hydrocarbon = id_string("C=C"),
	Tritium = id_string("T"),
	Explosive = id_string("expl"),

	// Structures
	Core = id_string("core"),

	// Walls
	ConcreteWall = id_string("WL-C"),
	AlloyWall = id_string("WL-A"),

	// Extractors
	LaserDrill = id_string("DR-L"),
	AlloyDrill = id_string("DR-A"),
	ExplodeDrill = id_string("DR-E"),
	LakePump = id_string("PP-W"),
	OceanPump = id_string("PP-S"),
	AlgaeCollector = id_string("ALC"),

	// Power systems
	SolarPanel = id_string("SP"),
	SteamGenerator = id_string("SG"),
	DifferentialGenerator = id_string("DG"),
	WaveGenerator = id_string("WG"),
	RTG = id_string("RTG"),
	FusionReactor = id_string("FR"),
	Battery = id_string("BATT"),

	// Turrets
	LaserTurret = id_string("LSLS"),

	// Devices
	BasicVehicleChassis = id_string("VC-B"),
	PoweredVehicleChassis = id_string("VC-P"),
};

} // namespace istd

#endif
