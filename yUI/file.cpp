#include <file.h>
#include <json.h>

#include <GameData.h>
#include <GameAPI.h>
#include <GameRTTI.h>
#include <Utilities.h>

#include <SortingIcons.h>

#include <filesystem>
#include <fstream>

namespace SIFiles
{
	void JSONEntryItemRecursiveEmplace(const JSONEntryItemCommon&, TESForm*);

	void HandleSIJson(const std::filesystem::path& path)
	{
		Log("\nReading from JSON file " + path.string());
		constexpr auto strToFormID = [](const std::string& formIdStr)
		{
			const auto formId = HexStringToInt(formIdStr);
			if (formId == -1) Log("Form field was incorrectly formatted, got " + formIdStr);
			return formId;
		};
		try
		{
			std::ifstream i(path);
			nlohmann::json j;
			j = nlohmann::json::parse(i, nullptr, true, true);
			if (j.contains("tags") && j["tags"].is_array())
			{
				for (auto& elem : j["tags"])
				{
					if (!elem.is_object())
					{
						Log("JSON error: expected object with mod, form and folder fields");
						continue;
					}
					JSONEntryItemCommon common{};

					common.tag = elem["tag"].get<std::string>();
					if (elem.contains("priority")) common.priority = elem["priority"].get<SInt32>();
					if (elem.contains("formType")) common.formType = elem["formType"].get<UInt32>();
					if (elem.contains("questItem")) common.questItem = elem["questItem"].get<UInt8>();
					if (elem.contains("miscComponent")) common.miscComponent = elem["miscComponent"].get<UInt8>();
					if (elem.contains("miscProduct")) common.miscProduct = elem["miscProduct"].get<UInt8>();

					if (elem.contains("mod") && elem.contains("form"))
					{
						auto modName = elem.contains("mod") ? elem["mod"].get<std::string>() : "";
						const auto mod = !modName.empty() ? DataHandler::GetSingleton()->LookupModByName(modName.c_str()) : nullptr;

						UInt8 modIndex;
						if (modName == "FF") modIndex = 0xFF;
						else if (!mod && !modName.empty())
						{
							Log("Mod name " + modName + " was not found");
							continue;
						}
						modIndex = mod->modIndex;

						UInt32 formlist = 0;
						if (elem.contains("formlist")) formlist = elem["formlist"].get<UInt32>();

						std::vector<int> formIds;
						if (const auto formElem = elem.contains("form") ? &elem["form"] : nullptr; formElem)
						{
							if (formElem->is_array())
								std::ranges::transform(*formElem, std::back_inserter(formIds), [&](auto& i) {return strToFormID(i.template get<std::string>()); });
							else
								formIds.push_back(strToFormID(formElem->get<std::string>()));
							if (std::ranges::find(formIds, -1) != formIds.end())
								continue;
						}

						if (mod && !formIds.empty())
						{
							for (auto formId : formIds)
							{
								formId = (modIndex << 24) + (formId & 0x00FFFFFF);
								common.form = LookupFormByID(formId);
								if (!common.form) { Log(FormatString("Form %X was not found", formId)); continue; }
								if (!formlist) {
									Log(FormatString("Tag: '%10s', form: %08X (%50s), individual", common.tag.c_str(), formId, common.form->GetName()));
									g_Items_JSON.emplace_back(common);
								}
								else if (formlist == 1) {
									JSONEntryItemRecursiveEmplace(common, common.form);
								}
								else if (formlist == 2) {
									for (const auto item : *GetAllForms()) {
										if (item->typeID == kFormType_TESObjectWEAP) {
											const auto weapon = DYNAMIC_CAST(item, TESForm, TESObjectWEAP);
											if (!weapon || !weapon->repairItemList.listForm) continue;
											if (weapon->refID == common.form->refID || weapon->repairItemList.listForm->refID == common.form->refID || weapon->repairItemList.listForm->ContainsRecursive(common.form)) {
												Log(FormatString("Tag: '%10s', form: %08X (%50s), recursive, repair list: '%08X'", common.tag.c_str(), item->refID, item->GetName(), formId));
												g_Items_JSON.emplace_back(common, item);
											}
										} else if (item->typeID == kFormType_TESObjectARMO) {
											const auto armor = DYNAMIC_CAST(item, TESForm, TESObjectARMO);
											if (!armor || !armor->repairItemList.listForm) continue;
											if (armor->refID == common.form->refID || armor->repairItemList.listForm->refID == common.form->refID || armor->repairItemList.listForm->ContainsRecursive(common.form)) {
												Log(FormatString("Tag: '%10s', form: %08X (%50s), recursive, repair list: '%08X'", common.tag.c_str(), item->refID, item->GetName(), formId));
												g_Items_JSON.emplace_back(common, item);
											}
										}
									}
								}
							}
						}
						else {
							Log(FormatString("Tag: '%10s', mod: '%s'", common.tag.c_str(), modName.c_str()));
							g_Items_JSON.emplace_back(common);
						}
					}
					else if (common.formType == 40 || elem.contains("weaponSkill") || elem.contains("weaponType"))
					{
						common.formType = 40;
						JSONEntryItemWeapon weapon{};

						if (elem.contains("weaponSkill"))			weapon.weaponSkill = elem["weaponSkill"].get<UInt32>();
						if (elem.contains("weaponHandgrip"))		weapon.weaponHandgrip = elem["weaponHandgrip"].get<UInt32>();
						if (elem.contains("weaponAttackAnim"))		weapon.weaponAttackAnim = elem["weaponAttackAnim"].get<UInt32>();
						if (elem.contains("weaponReloadAnim"))		weapon.weaponReloadAnim = elem["weaponReloadAnim"].get<UInt32>();
						if (elem.contains("weaponIsAutomatic"))		weapon.weaponIsAutomatic = elem["weaponIsAutomatic"].get<UInt32>();
						if (elem.contains("weaponHasScope"))		weapon.weaponHasScope = elem["weaponHasScope"].get<UInt32>();
						if (elem.contains("weaponIgnoresDTDR"))		weapon.weaponIgnoresDTDR = elem["weaponIgnoresDTDR"].get<UInt32>();
						if (elem.contains("weaponClipRounds"))		weapon.weaponClipRounds = elem["weaponClipRounds"].get<UInt32>();
						if (elem.contains("weaponNumProjectiles"))	weapon.weaponNumProjectiles = elem["weaponNumProjectiles"].get<UInt32>();
						if (elem.contains("weaponSoundLevel"))		weapon.weaponSoundLevel = elem["weaponSoundLevel"].get<UInt32>();

						if (elem.contains("ammoMod") && elem.contains("ammoForm"))
						{
							auto ammoModName = elem["ammoMod"].get<std::string>();
							const auto ammoMod = !ammoModName.empty() ? DataHandler::GetSingleton()->LookupModByName(ammoModName.c_str()) : nullptr;
							auto ammoForm = LookupFormByID((ammoMod->modIndex << 24) + (strToFormID(elem["ammoForm"].get<std::string>()) & 0x00FFFFFF));
							weapon.ammo = ammoForm;
						}

						std::vector<int> weaponTypes;
						if (auto* weaponTypeElem = elem.contains("weaponType") ? &elem["weaponType"] : nullptr; weaponTypeElem)
						{
							if (weaponTypeElem->is_array())
								std::ranges::transform(*weaponTypeElem, std::back_inserter(weaponTypes), [&](auto& i) {return i.template get<UInt32>(); });
							else
								weaponTypes.push_back(weaponTypeElem->get<UInt32>());
							if (std::ranges::find(weaponTypes, -1) != weaponTypes.end())
								continue;
						}

						if (!weaponTypes.empty())
						{
							for (auto weaponType : weaponTypes)
							{
								weapon.weaponType = weaponType;
								Log(FormatString("Tag: '%10s', weapon condition, type: %d", common.tag.c_str(), weaponType));
								g_Items_JSON.emplace_back(common, weapon);
							}
						}
						else {
							Log(FormatString("Tag: '%10s', weapon condition", common.tag.c_str()));
							g_Items_JSON.emplace_back(common, weapon);
						}
					}
					else if (common.formType == 24)
					{
						common.formType = 24;
						JSONEntryItemArmor armor{};

						if (elem.contains("armorHead")) 		(elem["armorHead"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1;
						if (elem.contains("armorHair")) 		(elem["armorHair"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 1;
						if (elem.contains("armorUpperBody")) 	(elem["armorUpperBody"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 2;
						if (elem.contains("armorLeftHand")) 	(elem["armorLeftHand"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 3;
						if (elem.contains("armorRightHand")) 	(elem["armorRightHand"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 4;
						if (elem.contains("armorWeapon")) 		(elem["armorWeapon"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 5;
						if (elem.contains("armorPipBoy")) 		(elem["armorPipBoy"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 6;
						if (elem.contains("armorBackpack")) 	(elem["armorBackpack"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 7;
						if (elem.contains("armorNecklace")) 	(elem["armorNecklace"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 8;
						if (elem.contains("armorHeadband")) 	(elem["armorHeadband"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 9;
						if (elem.contains("armorHat")) 			(elem["armorHat"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 10;
						if (elem.contains("armorEyeglasses")) 	(elem["armorEyeglasses"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 11;
						if (elem.contains("armorNosering")) 	(elem["armorNosering"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 12;
						if (elem.contains("armorEarrings")) 	(elem["armorEarrings"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 13;
						if (elem.contains("armorMask")) 		(elem["armorMask"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 14;
						if (elem.contains("armorChoker")) 		(elem["armorChoker"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 15;
						if (elem.contains("armorMouthObject"))	(elem["armorMouthObject"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 16;
						if (elem.contains("armorBodyAddon1"))	(elem["armorBodyAddon1"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 17;
						if (elem.contains("armorBodyAddon2"))	(elem["armorBodyAddon2"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 18;
						if (elem.contains("armorBodyAddon3"))	(elem["armorBodyAddon3"].get<SInt8>() == 1 ? armor.armorSlotsMaskWL : armor.armorSlotsMaskBL) += 1 << 19;


						if (elem.contains("armorClass"))		armor.armorClass = elem["armorClass"].get<UInt16>();
						if (elem.contains("armorPower"))		armor.armorPower = elem["armorPower"].get<SInt8>();
						if (elem.contains("armorHasBackpack"))	armor.armorHasBackpack = elem["armorHasBackpack"].get<SInt8>();

						if (elem.contains("armorDT"))			armor.armorDT = elem["armorDT"].get<float>();
						if (elem.contains("armorDR"))			armor.armorDR = elem["armorDR"].get<UInt16>();
						if (elem.contains("armorChangesAV"))	armor.armorChangesAV = elem["armorChangesAV"].get<UInt16>();

						g_Items_JSON.emplace_back(common, armor);
					}
					else if (common.formType == 31 || elem.contains("miscComponent"))
					{
						common.formType = 31;
						JSONEntryItemMisc misc{};
						g_Items_JSON.emplace_back(common, misc);
					}
					else if (common.formType == 47 || elem.contains("IsFood") || elem.contains("IsMedicine"))
					{
						common.formType = 47;
						JSONEntryItemAid aid{};

						if (elem.contains("aidRestoresAV"))			aid.aidRestoresAV = elem["aidRestoresAV"].get<UInt8>();
						if (elem.contains("aidDamagesAV"))			aid.aidDamagesAV = elem["aidDamagesAV"].get<UInt8>();
						if (elem.contains("aidIsAddictive"))		aid.aidIsAddictive = elem["aidIsAddictive"].get<UInt8>();
						if (elem.contains("aidIsFood"))				aid.aidIsFood = elem["aidIsFood"].get<UInt8>();
						if (elem.contains("aidIsWater"))			aid.aidIsWater = elem["aidIsWater"].get<UInt8>();
						if (elem.contains("aidIsMedicine"))			aid.aidIsMedicine = elem["aidIsMedicine"].get<UInt8>();
						if (elem.contains("aidIsPoisonous"))		aid.aidIsPoisonous = elem["aidIsPoisonous"].get<UInt8>();
						g_Items_JSON.emplace_back(common, aid);
					}
					else g_Items_JSON.emplace_back(common);
				}
			}
			else { Log(path.string() + " JSON tag array not detected"); }

			if (j.contains("icons") && j["icons"].is_array())
			{
				for (auto& elem : j["icons"])
				{
					if (!elem.is_object())
					{
						Log("JSON error: expected object with mod, form and folder fields");
						continue;
					}

					const auto& tag = elem["tag"].get<std::string>();

					SInt16 priority = 0;
					if (elem.contains("priority")) priority = elem["priority"].get<SInt16>();

					std::string xmltemplate;
					if (elem.contains("template")) xmltemplate = elem["template"].get<std::string>();
					std::string filename;
					if (elem.contains("filename")) filename = elem["filename"].get<std::string>();
					std::string texatlas;
					if (elem.contains("texatlas")) texatlas = elem["texatlas"].get<std::string>();
					SInt64 systemcolor = 0;
					if (elem.contains("systemcolor")) systemcolor = elem["systemcolor"].get<SInt32>();
					std::string category;
					if (elem.contains("category")) category = elem["category"].get<std::string>();
					std::string name;
					if (elem.contains("name")) name = UTF8toANSI(elem["name"].get<std::string>());
					std::string icon;
					if (elem.contains("icon")) icon = elem["icon"].get<std::string>();
					UInt32 tab = 0;
					if (elem.contains("tab")) tab = elem["tab"].get<UInt32>();
					UInt32 count = 0;
					if (elem.contains("count")) count = elem["count"].get<UInt32>();

					Log(FormatString("Tag: '%10s', icon: '%s'", tag.c_str(), filename.c_str()));
					g_Tags_JSON.emplace_back(tag, priority, xmltemplate, filename, texatlas, systemcolor, category, name, icon, tab, count);
				}
			}
			else { Log(path.string() + " JSON icon array not detected"); }
		}
		catch (nlohmann::json::exception& e)
		{
			Log("The JSON is incorrectly formatted! It will not be applied.");
			Log(FormatString("JSON error: %s\n", e.what()));
		}

	}

	void JSONEntryItemRecursiveEmplace(const JSONEntryItemCommon& common, TESForm* item)
	{
		if (item->typeID == kFormType_BGSListForm)
		{
			const auto bgslist = DYNAMIC_CAST(item, TESForm, BGSListForm);
			if (!bgslist) return;
			for (const auto iter : bgslist->list)
				if (iter) JSONEntryItemRecursiveEmplace(common, iter);
		} else if (!common.formType || item->typeID == common.formType) {
			Log(FormatString("Tag: '%10s', form: %08X (%50s), recursive", common.tag.c_str(), item->refID, item->GetName()));
			g_Items_JSON.emplace_back(common, item);
		}
	}
}

void LoadSIMapsFromFiles()
{
	Log("Loading files");
	const auto dir = GetCurPath() + R"(\Data\menus\ySI)";
	const auto then = std::chrono::system_clock::now();
	if (std::filesystem::exists(dir))
	{
		for (std::filesystem::directory_iterator iter(dir.c_str()), end; iter != end; ++iter)
		{
			const auto& path = iter->path();
			const auto& fileName = path.filename();
			if (iter->is_directory())
				Log(iter->path().string() + " found");
			else if (_stricmp(path.extension().string().c_str(), ".json") == 0)
				SIFiles::HandleSIJson(iter->path());
			else if (_stricmp(path.extension().string().c_str(), ".xml") == 0)
			{
				auto pathstring = iter->path().generic_string();
				auto relativepath = pathstring.substr(pathstring.find_last_of("\\Data\\") - 3);
				SI::g_XMLPaths.emplace_back(std::filesystem::path(relativepath));
			}
		}
	}
	else
	{
		Log(dir + " does not exist.");
	}
	SIFiles::FillSIMapsFromJSON();
	const auto now = std::chrono::system_clock::now();
	const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - then);
	Log(FormatString("Loaded tags and icons in %d ms", diff.count()));
}