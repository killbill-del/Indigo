/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "molecule/structure_checker2.h"
#include "api/indigo.h"
#include "api/src/indigo_molecule.h"
#include "api/src/indigo_reaction.h"
#include "base_cpp/tlscont.h"
#include "molecule/molecule_automorphism_search.h"
#include "third_party/rapidjson/stringbuffer.h"
#include "third_party/rapidjson/writer.h"
#include <functional>
#include <regex>
#include <string>

using namespace indigo;

struct CheckParams
{
    int check_flags = StructureChecker2::CHECK_ALL;
    std::vector<int> selected_atoms;
    std::vector<int> selected_bonds;
};

static CheckParams check_params_from_string(const char* params)
{
    CheckParams r;
    size_t len = sizeof(StructureChecker2::checkTypeName) / sizeof(*StructureChecker2::checkTypeName);
    if (params)
    {
        std::smatch sm1;
        std::unordered_set<std::string> words;
        std::string s(params);
        std::regex rx1(R"(\b(\w+)\b)", std::regex_constants::icase);
        while (std::regex_search(s, sm1, rx1))
        {
            words.insert(sm1[1]);
            s = sm1.suffix();
        }
        r.check_flags = StructureChecker2::CHECK_NONE;
        for (int i = 0; i < len; i++)
        {
            if (words.find(std::string(StructureChecker2::checkTypeName[i])) != words.end())
            {
                r.check_flags |= 1 << i;
            }
        }
        r.check_flags = !r.check_flags || r.check_flags & 1 << (len - 1) ? StructureChecker2::CHECK_ALL
                                                                         : (r.check_flags == 1 ? StructureChecker2::CHECK_NONE : r.check_flags >> 1);

        std::smatch sm2;
        s = params;
        std::regex rx2(R"(\b(atoms|bonds)\b((?:\W+\b\d+\b\W*?)+))", std::regex_constants::icase);
        std::regex rx3(R"(\b(\d+)\b)");
        std::smatch sm3;
        while (std::regex_search(s, sm2, rx2))
        {
            std::vector<int>& vec = std::tolower(sm2[1].str()[0]) == 'a' ? r.selected_atoms : r.selected_bonds;
            std::string a = sm2[2];
            while (std::regex_search(a, sm3, rx3))
            {
                vec.push_back(atoi(sm3[1].str().c_str()));
                a = sm3.suffix();
            }
            s = sm2.suffix();
        }
    }
    return r;
}
static void checkMolecule(const IndigoObject& bmol, int check_types, const std::vector<int>& selected_atoms, const std::vector<int>& selected_bonds,
                          StructureChecker2::CheckResult& result);
static void checkMolecule(BaseMolecule& item, int check_types, const std::vector<int>& selected_atoms, const std::vector<int>& selected_bonds,
                          StructureChecker2::CheckResult& result);

static const std::string& message_text(StructureChecker2::CheckMessageCode code);

StructureChecker2::StructureChecker2()
{
}

StructureChecker2::CheckResult StructureChecker2::check(const char* item, const char* check_flags, const char* load_params)
{
    int it = indigoLoadStructureFromString(item, load_params);
    if (it < 0)
    {
        it = indigoLoadStructureFromString(item, (std::string(load_params ? load_params : "") + " query").c_str()); //##!!!PATCH
    }
    auto r = check(it, check_flags);
    indigoFree(it);
    return r;
}

StructureChecker2::CheckResult StructureChecker2::check(int item, const char* check_flags)
{
    auto p = check_params_from_string(check_flags);
    return check(item, p.check_flags, p.selected_atoms, p.selected_bonds);
}

StructureChecker2::CheckResult StructureChecker2::check(int item, int check_types, const std::vector<int>& selected_atoms,
                                                        const std::vector<int>& selected_bonds)
{
    if (item < 0)
    {
        CheckResult r;
        r.message(StructureChecker2::CheckMessageCode::CHECK_MSG_LOAD);
        return r;
    }
    else
    {
        return check(indigoGetInstance().getObject(item), check_types, selected_atoms, selected_bonds);
    }
}

StructureChecker2::CheckResult StructureChecker2::check(const IndigoObject& item, int check_types, const std::vector<int>& selected_atoms,
                                                        const std::vector<int>& selected_bonds)
{
    CheckResult r;
    if (IndigoBaseMolecule::is((IndigoObject&)item))
    {
        checkMolecule(item, check_types, selected_atoms, selected_bonds, r);
    }
    else if (IndigoBaseReaction::is((IndigoObject&)item))
    {
        BaseReaction& reaction = ((IndigoObject&)item).getBaseReaction();
        bool query = reaction.isQueryReaction();
        BaseReaction* brxn = query ? (BaseReaction*)&reaction.asQueryReaction() : (BaseReaction*)&reaction.asReaction();

#define CHECK_REACTION_COMPONENT(KIND)                                                                                                                         \
    for (auto i = brxn->KIND##Begin(); i < brxn->KIND##End(); i = brxn->KIND##Next(i))                                                                         \
    {                                                                                                                                                          \
        CheckResult res;                                                                                                                                       \
        checkMolecule(brxn->getBaseMolecule(i), check_types, selected_atoms, selected_bonds, res);                                                             \
        if (!res.isEmpty())                                                                                                                                    \
        {                                                                                                                                                      \
            r.message(StructureChecker2::CheckMessageCode::CHECK_MSG_REACTION, i, res);                                                                        \
        }                                                                                                                                                      \
    }
        CHECK_REACTION_COMPONENT(reactant)
        CHECK_REACTION_COMPONENT(product)
        CHECK_REACTION_COMPONENT(catalyst)
#undef CHECK_REACTION_COMPONENT
    }
    else if (IndigoAtom::is((IndigoObject&)item))
    {
        IndigoAtom& ia = IndigoAtom::cast((IndigoObject&)item);
        std::vector<int> atoms = {ia.getIndex() + 1};
        checkMolecule(ia.mol, check_types, atoms, std::vector<int>(), r);
    }
    else if (IndigoBond::is((IndigoObject&)item))
    {
        IndigoBond& ib = IndigoBond::cast((IndigoObject&)item);
        std::vector<int> bonds = {ib.getIndex() + 1};
        checkMolecule(ib.mol, check_types, std::vector<int>(), bonds, r);
    }
    return r;
}

bool StructureChecker2::CheckResult::isEmpty() const
{
    return messages.size() == 0;
}

void StructureChecker2::CheckResult::message(CheckMessageCode code, int index, const CheckResult& subresult)
{
    CheckMessage m = {code, message_text(code), index, std::vector<int>(), subresult};
    messages.push_back(m);
}

void StructureChecker2::CheckResult::message(CheckMessageCode code, const std::vector<int>& ids)
{
    CheckMessage m = {code, message_text(code), -1, ids, CheckResult()};
    messages.push_back(m);
}

void indigo::StructureChecker2::CheckResult::message(CheckMessageCode code, const std::unordered_set<int>& ids)
{
    std::vector<int> v;
    std::copy(ids.begin(), ids.end(), std::back_inserter(v));
    message(code, v);
}

void StructureChecker2::CheckResult::message(CheckMessageCode code)
{
    CheckMessage m = {code, message_text(code), -1, std::vector<int>(), CheckResult()};
    messages.push_back(m);
}

using namespace rapidjson;
static void _toJson(const StructureChecker2::CheckResult& data, Writer<StringBuffer>& writer)
{
    writer.StartArray();
    for (auto msg : data.messages)
    {
        writer.StartObject();
        writer.Key("code");
        writer.Uint(static_cast<unsigned int>(msg.code));
        writer.Key("message");
        writer.String(msg.message.c_str());
        if (msg.index >= 0)
        {
            writer.Key("index");
            writer.Uint(msg.index);
        }
        if (!msg.ids.empty())
        {
            writer.Key("ids");
            writer.StartArray();
            for (auto i : msg.ids)
            {
                writer.Uint(i);
            }
            writer.EndArray();
        }
        if (!msg.subresult.isEmpty())
        {
            writer.Key("subresult");
            _toJson(msg.subresult, writer);
        }
        writer.EndObject();
    }
    writer.EndArray();
}
std::string StructureChecker2::CheckResult::toJson()
{
    std::stringstream result;
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    _toJson(*this, writer);
    return std::string(s.GetString());
}

/**************************************************/
static void filter_atoms(Molecule& mol, const std::unordered_set<int>& selected_atoms, StructureChecker2::CheckResult& result,
                         StructureChecker2::CheckMessageCode msg, const std::function<bool(Molecule&, int)>& filter, bool default_filter = true)
{
    std::vector<int> ids;
    std::copy_if(selected_atoms.begin(), selected_atoms.end(), std::back_inserter(ids), [&mol, &filter, default_filter](int idx) {
        return (!default_filter || !mol.isPseudoAtom(idx) && !mol.isRSite(idx) && !mol.isTemplateAtom(idx)) && filter(mol, idx);
    });
    if (ids.size())
    {
        result.message(msg, ids);
    }
}
//
static void check_load(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                       StructureChecker2::CheckResult& result)
{
    if (mol.vertexCount() == 0)
    {
        result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_EMPTY);
    }
}

#define FILTER_ATOMS(MSG, FILTER) filter_atoms(mol, selected_atoms, result, MSG, FILTER, false);
#define FILTER_ATOMS_DEFAULT(MSG, FILTER) filter_atoms(mol, selected_atoms, result, MSG, FILTER);

static void check_valence(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                          StructureChecker2::CheckResult& result)
{

    if (mol.isQueryMolecule())
    {
        result.message(
            StructureChecker2::CheckMessageCode::CHECK_MSG_VALENCE_NOT_CHECKED_QUERY); // 'Structure contains query features, so valency could not be checked'
    }
    else if (mol.hasRGroups())
    {
        result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_VALENCE_NOT_CHECKED_RGROUP); // 'Structure contains RGroup components, so valency could
    }
    else
    {
        FILTER_ATOMS(StructureChecker2::CheckMessageCode::CHECK_MSG_VALENCE, [](Molecule& mol, int idx) { return mol.getAtomValence_NoThrow(idx, -1) == -1; });
    }
}

static void check_radical(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                          StructureChecker2::CheckResult& result)
{
    if (mol.hasPseudoAtoms())
    {
        result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_RADICAL_NOT_CHECKED_PSEUDO);
    }
    else
    {
        FILTER_ATOMS_DEFAULT(StructureChecker2::CheckMessageCode::CHECK_MSG_RADICAL,
                             [](Molecule& mol, int idx) { return mol.getAtomRadical_NoThrow(idx, -1) > 0; });
    }
}
static void check_pseudoatom(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                             StructureChecker2::CheckResult& result)
{
    FILTER_ATOMS(StructureChecker2::CheckMessageCode::CHECK_MSG_PSEUDOATOM, [](Molecule& mol, int idx) { return mol.isPseudoAtom(idx); });
}
static void check_stereo(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker2::CheckResult& result)
{
    if (!mol.isQueryMolecule())
    {
        Molecule target;
        auto saved_valence_flag = mol.asMolecule().getIgnoreBadValenceFlag();
        mol.asMolecule().setIgnoreBadValenceFlag(true);
        target.clone_KeepIndices(mol);

        for (auto i : target.vertices())
        {
            if (!target.stereocenters.exists(i) && target.stereocenters.isPossibleStereocenter(i))
            {
                try
                {
                    target.stereocenters.add(i, MoleculeStereocenters::ATOM_ABS, 0, false);
                }
                catch (Exception& e)
                {
                    e.code();
                    // Just ignore this stereo center
                }
            }
        }

        MoleculeAutomorphismSearch as;

        as.detect_invalid_cistrans_bonds = true;
        as.detect_invalid_stereocenters = true;
        as.find_canonical_ordering = false;
        as.process(target);

        for (auto i : target.vertices())
        {
            if (target.stereocenters.exists(i) && as.invalidStereocenter(i))
            {
                target.stereocenters.remove(i);
            }
        }

        FILTER_ATOMS_DEFAULT(StructureChecker2::CheckMessageCode::CHECK_MSG_3D_STEREO, [](Molecule& mol, int idx) {
            bool stereo_3d = true;
            if (BaseMolecule::hasZCoord(mol) && mol.stereocenters.exists(idx))
            {
                const Vertex& vertex = mol.getVertex(idx);
                for (auto j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
                    if (mol.getBondDirection2(idx, vertex.neiVertex(j)) > 0)
                        stereo_3d = false;
            }
            return stereo_3d;
        });

        FILTER_ATOMS_DEFAULT(StructureChecker2::CheckMessageCode::CHECK_MSG_WRONG_STEREO, [&target](Molecule& mol, int idx) {
            return mol.stereocenters.exists(idx) && target.stereocenters.exists(idx) && mol.stereocenters.getType(idx) != target.stereocenters.getType(idx) ||
                   mol.stereocenters.exists(idx) && !target.stereocenters.exists(idx);
        });
        FILTER_ATOMS_DEFAULT(StructureChecker2::CheckMessageCode::CHECK_MSG_UNDEFINED_STEREO,
                             [&target](Molecule& mol, int idx) { return !mol.stereocenters.exists(idx) && target.stereocenters.exists(idx); });

        mol.asMolecule().setIgnoreBadValenceFlag(saved_valence_flag);
    }
}
static void check_query(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                        StructureChecker2::CheckResult& result)
{
    if (mol.isQueryMolecule())
    {
        result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_QUERY);
    }
    FILTER_ATOMS_DEFAULT(StructureChecker2::CheckMessageCode::CHECK_MSG_QUERY_ATOM,
                         [](Molecule& mol, int idx) { return mol.reaction_atom_exact_change[idx] || mol.reaction_atom_inversion[idx]; });

    std::vector<int> ids;
    std::copy_if(selected_atoms.begin(), selected_atoms.end(), std::back_inserter(ids),
                 [&mol](int idx) { return mol.reaction_bond_reacting_center[idx] != 0; });
    if (ids.size())
    {
        result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_QUERY_BOND, ids);
    }
}

static float calc_mean_dist(Molecule& mol)
{
    float mean_dist = 0.0;
    for (auto i : mol.edges())
    {
        const Edge& edge = mol.getEdge(i);
        Vec3f& a = mol.getAtomXyz(edge.beg);
        Vec3f& b = mol.getAtomXyz(edge.end);
        mean_dist += Vec3f::dist(a, b);
    }
    if (mol.edgeCount() > 0)
        mean_dist = mean_dist / mol.edgeCount();
    return mean_dist;
}

static void check_overlap_atom(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                               StructureChecker2::CheckResult& result)
{
    auto mean_dist = calc_mean_dist(mol);
    std::unordered_set<int> ids;
    std::for_each(selected_atoms.begin(), selected_atoms.end(), [&mol, &ids, mean_dist](int idx) {
        Vec3f& a = mol.getAtomXyz(idx);
        for (auto i : mol.vertices())
        {
            if (i != idx)
            {
                Vec3f& b = mol.getAtomXyz(i);
                if ((mean_dist > 0.0) && (Vec3f::dist(a, b) < 0.25 * mean_dist))
                {
                    ids.insert(idx);
                    ids.insert(i);
                }
            }
        }
    });
    result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_OVERLAP_ATOM, ids);
}
static void check_overlap_bond(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                               StructureChecker2::CheckResult& result)
{
    if (BaseMolecule::hasCoord(mol))
    {
        auto mean_dist = calc_mean_dist(mol);
        std::unordered_set<int> ids;
        std::for_each(selected_bonds.begin(), selected_bonds.end(), [&mol, &ids, mean_dist](int idx) {
            const Edge& e1 = mol.getEdge(idx);
            Vec2f a1, b1, a2, b2;
            Vec2f::projectZ(a1, mol.getAtomXyz(e1.beg));
            Vec2f::projectZ(b1, mol.getAtomXyz(e1.end));

            for (auto i : mol.edges())
            {
                if ((i != idx))
                {
                    const Edge& e2 = mol.getEdge(i);
                    Vec2f::projectZ(a2, mol.getAtomXyz(e2.beg));
                    Vec2f::projectZ(b2, mol.getAtomXyz(e2.end));
                    if ((Vec2f::dist(a1, a2) < 0.01 * mean_dist) || (Vec2f::dist(b1, b2) < 0.01 * mean_dist) || (Vec2f::dist(a1, b2) < 0.01 * mean_dist) ||
                        (Vec2f::dist(b1, a2) < 0.01 * mean_dist))
                        continue;

                    if (Vec2f::segmentsIntersect(a1, b1, a2, b2))
                    {
                        ids.insert(idx);
                        ids.insert(i);
                    }
                }
            }
        });
    }
}

static void check_rgroup(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker2::CheckResult& result)
{
    if (mol.hasRGroups())
    {
        result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_RGROUP);
    }
}
static void check_sgroup(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker2::CheckResult& result)
{
}
static void check_tgroup(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker2::CheckResult& result)
{
    if (mol.tgroups.getTGroupCount() > 0)
    {
        result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_TGROUP);
    }
}
static void check_chirality(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                            StructureChecker2::CheckResult& result)
{
    // not impl
    result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_CHIRALITY);
}
static void check_chiral_flag(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                              StructureChecker2::CheckResult& result)
{
    if (mol.getChiralFlag() > 0 && mol.stereocenters.size() == 0)
    {
        result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_CHIRAL_FLAG);
    }
}
static void check_3d_coord(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                           StructureChecker2::CheckResult& result)
{
    FILTER_ATOMS(StructureChecker2::CheckMessageCode::CHECK_MSG_3D_COORD, [](Molecule& mol, int idx) { return fabs(mol.getAtomXyz(idx).z) > 0.001; });
}
static void check_charge(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                         StructureChecker2::CheckResult& result)
{
    // not impl
    result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_CHARGE_NOT_IMPL);
}
static void check_salt(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                       StructureChecker2::CheckResult& result)
{
    // not impl
    result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_SALT_NOT_IMPL);
}
static void check_ambigous_h(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                             StructureChecker2::CheckResult& result)
{
    if (mol.isQueryMolecule())
    {
        result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_AMBIGUOUS_H_NOT_CHECKED_QUERY);
    }
    else
    {
        FILTER_ATOMS_DEFAULT(StructureChecker2::CheckMessageCode::CHECK_MSG_AMBIGUOUS_H, [](Molecule& mol, int idx) {
            return mol.asMolecule().getImplicitH_NoThrow(idx, -1) == -1 && mol.getAtomAromaticity(idx) == ATOM_AROMATIC;
        });
    }
}
static void check_coord(Molecule& mol, const std::unordered_set<int>& selected_atoms, const std::unordered_set<int>& selected_bonds,
                        StructureChecker2::CheckResult& result)
{
    if (mol.vertexCount() > 1 && !BaseMolecule::hasCoord(mol))
    {
        result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_ZERO_COORD);
    }
}
static void check_v3000(int mol, StructureChecker2::CheckResult& result)
{
    const char* f = indigoMolfile(mol);
    if (f && std::string(f).find("V3000") != -1)
    {
        result.message(StructureChecker2::CheckMessageCode::CHECK_MSG_V3000);
    }
}
#undef FILTER_ATOMS
#undef FILTER_ATOMS_DEFAULT

static void (*check_type_checkers[])(Molecule&, const std::unordered_set<int>&, const std::unordered_set<int>&, StructureChecker2::CheckResult&) = {
    &check_load,         &check_valence,      &check_radical, &check_pseudoatom, &check_stereo,     &check_query,
    &check_overlap_atom, &check_overlap_bond, &check_rgroup,  &check_sgroup,     &check_tgroup,     &check_chirality,
    &check_chiral_flag,  &check_3d_coord,     &check_charge,  &check_salt,       &check_ambigous_h, &check_coord};

static void checkMolecule(const IndigoObject& item, int check_types, const std::vector<int>& selected_atoms, const std::vector<int>& selected_bonds,
                          StructureChecker2::CheckResult& result)
{
    checkMolecule(((IndigoObject&)item).getBaseMolecule(), check_types, selected_atoms, selected_bonds, result);
    check_v3000(item.id, result);
}

static void checkMolecule(BaseMolecule& bmol, int check_types, const std::vector<int>& selected_atoms, const std::vector<int>& selected_bonds,
                          StructureChecker2::CheckResult& result)
{
    auto num_atoms = bmol.vertexEnd();
    auto num_bonds = bmol.edgeEnd();

    std::unordered_set<int> sel_atoms(num_atoms);
    std::unordered_set<int> sel_bonds(num_bonds);

    if (selected_atoms.size() == 0 && selected_bonds.size() == 0)
    {
        for (auto i : bmol.vertices())
        {
            sel_atoms.insert(i);
        }

        for (auto i : bmol.edges())
        {
            sel_bonds.insert(i);
        }
    }
    else
    {
        std::copy_if(selected_atoms.begin(), selected_atoms.end(), std::inserter(sel_atoms, sel_atoms.begin()),
                     [num_atoms](int i) { return i >= 0 && i < num_atoms; });
        std::copy_if(selected_bonds.begin(), selected_bonds.end(), std::inserter(sel_bonds, sel_bonds.begin()),
                     [num_bonds](int i) { return i >= 0 && i < num_bonds; });
    }
    std::transform(selected_bonds.begin(), selected_bonds.end(), std::inserter(sel_atoms, sel_atoms.begin()), [&bmol](int i) { return bmol.getEdge(i).beg; });
    std::transform(selected_bonds.begin(), selected_bonds.end(), std::inserter(sel_atoms, sel_atoms.begin()), [&bmol](int i) { return bmol.getEdge(i).end; });

    Molecule& mol = ((BaseMolecule&)bmol).isQueryMolecule() ? (Molecule&)((BaseMolecule&)bmol).asQueryMolecule() : ((BaseMolecule&)bmol).asMolecule();
    for (int i = 0; i < sizeof(check_type_checkers) / sizeof(*check_type_checkers); i++)
    {
        if (check_types & (1 << i))
        {
            check_type_checkers[i](mol, sel_atoms, sel_bonds, result);
        }
    }
}

static const std::string message_list[] = {"",
                                           "Error at loading structure, wrong format found",
                                           "Structure contains atoms with unusuall valence",
                                           "Structure contains query features, so valency could not be checked",
                                           "Structure contains RGroup components, so valency could not be checked",
                                           "IGNORE_BAD_VALENCE flag is active",
                                           "Structure contains radicals",
                                           "Structure contains pseudoatoms, so radicals could not be checked",
                                           "Structure contains pseudoatoms",
                                           "Structure contains wrong chiral flag",
                                           "Structure contains incorrect stereochemistry",
                                           "Structure contains stereocenters defined by 3D coordinates",
                                           "Structure contains stereocenters with undefined stereo configuration",
                                           "CHECK_MSG_IGNORE_STEREO_ERROR",
                                           "Structure contains query features",
                                           "Structure contains query features for atoms",
                                           "Structure contains query features for bonds",
                                           "CHECK_MSG_IGNORE_QUERY_FEATURE",
                                           "Structure contains overlapping atoms",
                                           "Structure contains overlapping bonds.",
                                           "Structure contains R-groups",
                                           "Structure contains S-groups",
                                           "Structure contains SCSR templates",
                                           "Structure has non-zero charge",
                                           "Structure contains charged fragments (possible salt)",
                                           "Input structure is empty",
                                           "Structure contains ambiguous hydrogens",
                                           "Structure contains query features, so ambiguous H could not be checked",
                                           "Structure contains 3D coordinates",
                                           "Structure has no atoms coordinates"
                                           "Reaction component check result",
                                           "Not implemented yet: check chirality",
                                           "Not implemented yet: check charge",
                                           "Not implemented yet: check salt",
                                           "Structure supports only Molfile V3000"

};

static const std::string& message_text(StructureChecker2::CheckMessageCode code)
{
    return message_list[static_cast<size_t>(code)];
}
