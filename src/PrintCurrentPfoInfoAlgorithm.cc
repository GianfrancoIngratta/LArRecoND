/**
 *  @file   src/PrintCurrentPfoInfoAlgorithm.cc
 *
 *  @brief  Implementation of the 3D list preparation algorithm class.
 *
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"
#include "Helpers/MCParticleHelper.h"
#include "Pandora/PandoraInternal.h"
#include "larpandoracontent/LArObjects/LArMCParticle.h"

#include "PreProcessingThreeDAlgorithm.h"

#include "larpandoracontent/LArHelpers/LArClusterHelper.h"
#include "larpandoracontent/LArObjects/LArCaloHit.h"
#include "larpandoracontent/LArHelpers/LArPfoHelper.h"
#include "larpandoracontent/LArHelpers/LArGeometryHelper.h"
#include "larpandoracontent/LArHelpers/LArMonitoringHelper.h"

#include "Objects/MCParticle.h"
#include <fstream>
#include <iterator>
#include <type_traits>

#include "PrintCurrentPfoInfoAlgorithm.h"
#include "larpandoracontent/LArMonitoring/MCParticleMonitoringAlgorithm.h"
#include "larpandoracontent/LArObjects/LArTrackPfo.h"

using namespace pandora;

namespace lar_content
{

PrintCurrentPfoInfoAlgorithm::PrintCurrentPfoInfoAlgorithm() : 
  m_inputStageName{""},
  // hits info
  m_inputCaloHitList3DName{""},
  m_inputCaloHitListUName{""},
  m_inputCaloHitListVName{""},
  m_inputCaloHitListWName{""},
  m_inputCaloHitList2DName{""},
  m_mcCaloHitListName{""},
  // clusters info
  m_inputClusterListName3D{""},
  m_inputClusterListNameU{""},
  m_inputClusterListNameV{""},
  m_inputClusterListNameW{""},
  // MC particles
  m_mcParticleListNames{""}
{
}

  //------------------------------------------------------------------------------------------------------------------------------------------
std::ofstream PrintCurrentPfoInfoAlgorithm::pfoInfoOutputFile("pfos_info.txt");
std::ofstream PrintCurrentPfoInfoAlgorithm::MCparticleInfoOutputFile("mc_particle_info.txt");

std::map<std::string, int> PrintCurrentPfoInfoAlgorithm::AlgoExecutionCount;

StatusCode PrintCurrentPfoInfoAlgorithm::Run()
{

  std::cout << "Running PrintCurrentPfoInfoAlgorithm after STAGE : "<< m_inputStageName << "\n";

  PrintCurrentPfoInfoAlgorithm::AlgoExecutionCount[m_inputStageName]++;

  // print hits info ----------------------------------------------------------
  const CaloHitList *pCaloHitList3D{nullptr};
  PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, m_inputCaloHitList3DName, pCaloHitList3D));
  PrintCaloHitsInfo(pCaloHitList3D, m_inputCaloHitList3DName, m_inputStageName);

  const CaloHitList *pCaloHitListU{nullptr};
  PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, m_inputCaloHitListUName, pCaloHitListU));
  PrintCaloHitsInfo(pCaloHitListU, m_inputCaloHitListUName, m_inputStageName);

  const CaloHitList *pCaloHitListW{nullptr};
  PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, m_inputCaloHitListWName, pCaloHitListW));
  PrintCaloHitsInfo(pCaloHitListW, m_inputCaloHitListWName, m_inputStageName);

  const CaloHitList *pCaloHitListV{nullptr};
  PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, m_inputCaloHitListVName, pCaloHitListV));
  PrintCaloHitsInfo(pCaloHitListV, m_inputCaloHitListVName, m_inputStageName);

  const CaloHitList *pCaloHitList2D{nullptr};
  PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, m_inputCaloHitList2DName, pCaloHitList2D));
  PrintCaloHitsInfo(pCaloHitList2D, m_inputCaloHitList2DName, m_inputStageName);

  // print clusters info -----------------------------------------------------------
  const ClusterList *pClusterList3D{nullptr};

  PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, m_inputClusterListName3D, pClusterList3D));
  PrintClusterListInfo(pClusterList3D, m_inputClusterListName3D, m_inputStageName);

  const ClusterList *pClusterListU{nullptr};

  PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS,STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, m_inputClusterListNameU, pClusterListU));
  PrintClusterListInfo(pClusterListU, m_inputClusterListNameU, m_inputStageName);

  const ClusterList *pClusterListV{nullptr};

  PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, m_inputClusterListNameV, pClusterListV));
  PrintClusterListInfo(pClusterListV, m_inputClusterListNameV, m_inputStageName);

  const ClusterList *pClusterListW{nullptr};

  PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, m_inputClusterListNameW, pClusterListW));
  PrintClusterListInfo(pClusterListW, m_inputClusterListNameW, m_inputStageName);
  
  // print true particles info -----------------------------------------------------------
  const MCParticleList *pMCParticleList = nullptr;
  PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, m_mcParticleListNames, pMCParticleList));
  PrintMCparticlesInfo(pMCParticleList);

  const CaloHitList *pMCCaloHitList = nullptr;
  PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, m_mcCaloHitListName, pMCCaloHitList));
  PrintMCparticlesInfo(pMCParticleList, pMCCaloHitList);


  // print pfo info --------------------------------------------------------------------
  for (unsigned int i = 0; i < m_inputPfoListNames.size(); ++i)
  {
    const std::string pfoListName(m_inputPfoListNames.at(i));
    const PfoList *pPfoList{nullptr};

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, pfoListName, pPfoList));

    if (!pPfoList || pPfoList->empty())
      continue;

    for (const ParticleFlowObject *pPfo : (*pPfoList))
      PrintPfoInfo(pPfo, m_inputStageName, pfoListName);

   }

  // pfoInfoOutputFile.close();
  pfoInfoOutputFile.flush();
  MCparticleInfoOutputFile.flush();
  
  // print CR Vertices info --------------------------------------------------------------------
  // for (unsigned int i = 0; i < m_inputVertexListNames.size(); ++i)
  // {
  //   const std::string VertexListName(m_inputVertexListNames.at(i));
  //   const VertexList *pVertexList{nullptr};
  //
  //   PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_INITIALIZED, !=, PandoraContentApi::GetList(*this, VertexListName, pVertexList));
  //
  //   if (!pVertexList || pVertexList->empty())
  //     continue;
  //
  //   for (const Vertex *vertex : (*pVertexList))
  //   {
  //     std::cout << "{ \"label\" : " << vertex->GetVertexLabel() 
  //               << ", \"type\" : " << vertex->GetVertexType()
  //               << ", \"X0\" : " << vertex->GetX0()
  //               << ", \"x\" : " << vertex->GetPosition().GetX() 
  //               << ", \"y\" : " << vertex->GetPosition().GetY() 
  //               << ", \"z\" : " << vertex->GetPosition().GetZ() 
  //               << "}\n";
  //   }
  //
  // }

  return STATUS_CODE_SUCCESS;
}

void PrintCurrentPfoInfoAlgorithm::PrintPfoInfo(const ParticleFlowObject *const pPfo,
    const std::string &STAGE, const std::string &LIST_NAME)
{
    if (!pfoInfoOutputFile.is_open())
    {
        std::cout << "Warning: pfoInfoOutputFile not open \n";
        return;
    }

    CaloHitList caloHitList3D;
    LArPfoHelper::GetCaloHits(pPfo, TPC_3D, caloHitList3D);

    int isClearCosmic = -999;
    try {
        isClearCosmic = static_cast<int>(pPfo->GetPropertiesMap().at("IsClearCosmic"));
    } catch (const std::out_of_range &) {
        // Key not found: keep default value (-999)
    }

    // > 1 true MC particle may contribute to this reco pfo -> consider
    // the one that has contributed the most to the pfo hits
    for (const auto& hit : caloHitList3D)
    {
        auto map = hit->GetMCParticleWeightMap();
        Uid mc_max_contrib = 0;
        float max_contrib = 0.;
        for (auto const &[this_mc, contrib] : map)
        {
            if (contrib > max_contrib)
            {
                mc_max_contrib = this_mc->GetUid();
                max_contrib = contrib;
            }
        }
        const long uid = reinterpret_cast<intptr_t>(mc_max_contrib);

        pfoInfoOutputFile
            << "{\"STAGE\" : \"" << STAGE << "\""
            << ", \"CALL\" : \"" << AlgoExecutionCount[m_inputStageName] << "\""
            << ", \"pfoListName\" : \"" << LIST_NAME << "\""
            << ", \"pfo\" : \"" << pPfo << "\""
            << ", \"mc_particle_uid\" : " << uid
            << ", \"isClearCosmic\" : \"" << isClearCosmic << "\""
            << ", \"CaloHitX\" : " << hit->GetPositionVector().GetX()
            << ", \"CaloHitY\" : " << hit->GetPositionVector().GetY()
            << ", \"CaloHitZ\" : " << hit->GetPositionVector().GetZ()
            << "},\n";
    }
}

void PrintCurrentPfoInfoAlgorithm::PrintCaloHitsInfo(const CaloHitList *& pCaloHitList, std::string HitsName, std::string STAGE)
{
  if(!pCaloHitList)
  { 
    // std::cout << "pCaloHitList from" << HitsName << "is nullptr\n";
    return;
  }

  if(!pfoInfoOutputFile.is_open())
  {
    std::cout << "Warning: pfoInfoOutputFile not open \n";
    return;
  }
  
  std::cout << "PrintCurrentPfoInfoAlgorithm::PrintCaloHitsInfo start \n";

  for (const CaloHit *const pCaloHit : *pCaloHitList)
  {

    auto map = pCaloHit->GetMCParticleWeightMap();

    Uid mc_max_contrib = 0;
    float max_contrib = 0.;
    
    for(auto const &[this_mc, contrib] : map)
    {
      if(max_contrib < contrib) {
            mc_max_contrib = this_mc->GetUid();
            max_contrib = contrib;
          }
    }
    
    // at the end of this loop we know the mc particle
    // that constribuited the most to this hit and 
    // store its relevant info

    const long uid = reinterpret_cast<intptr_t>(mc_max_contrib);

    pfoInfoOutputFile << "{\"STAGE\" : " << "\"" << STAGE << "\"" 
            << ", \"CallNumber\" : " << "\"" <<AlgoExecutionCount[m_inputStageName] << "\""
            << ", \"CaloHitType\" : " << "\""<< HitsName << "\""
            // << ", \"CaloHit\" : " << "\""<< pCaloHit << "\""
            << ", \"mc_particle_uid\" : " << uid
            << ", \"CaloHitX\" : " << pCaloHit->GetPositionVector().GetX() 
            << ", \"CaloHitY\" : " << pCaloHit->GetPositionVector().GetY()
            << ", \"CaloHitZ\" : " << pCaloHit->GetPositionVector().GetZ()
            << "}, \n";
  }

  std::cout << "PrintCurrentPfoInfoAlgorithm::PrintCaloHitsInfo end \n";
}

void PrintCurrentPfoInfoAlgorithm::PrintClusterListInfo(const ClusterList*& pClusterList, std::string clusterName, std::string STAGE)
{
  if(!pClusterList || pClusterList->empty())
  {
    // std::cout << __func__ << ": ClusterList " << clusterName << " is either nullptr or empty\n";
    return;
  }

  if(!pfoInfoOutputFile.is_open())
  {
    std::cout << "Warning: pfoInfoOutputFile not open \n";
    return;
  }

  std::cout << "PrintCurrentPfoInfoAlgorithm::PrintClusterListInfo start \n";

  for (const Cluster* pCluster : *pClusterList)
  {
    CaloHitList caloHitList;
    pCluster->GetOrderedCaloHitList().FillCaloHitList(caloHitList);

    for (const CaloHit *pCaloHit : caloHitList)
    {
      pfoInfoOutputFile << "{\"STAGE\" : " << "\"" << STAGE << "\"" 
              << ", \"CallNumber\" : " << "\"" <<AlgoExecutionCount[m_inputStageName] << "\""
              << ", \"ClusterName\" : " << "\"" << clusterName << "\"" 
              << ", \"Cluster\" : " << "\"" << pCluster << "\"" 
              << ", \"CaloHit\" : " << "\"" << pCaloHit << "\"" 
              << ", \"CaloHitX\" : " << pCaloHit->GetPositionVector().GetX() 
              << ", \"CaloHitY\" : " << pCaloHit->GetPositionVector().GetY()
              << ", \"CaloHitZ\" : " << pCaloHit->GetPositionVector().GetZ()
              << "}, \n";
    }
  }

  std::cout << "PrintCurrentPfoInfoAlgorithm::PrintClusterListInfo end \n";
  return;
}


//------------------------------------------------------------------------------------------------------------------------------------------
StatusCode PrintCurrentPfoInfoAlgorithm::PrintMCparticlesInfo(const MCParticleList*& pMCParticleList) const
{
    
    if(!pMCParticleList) return STATUS_CODE_NOT_INITIALIZED;

    for (const MCParticle* mcParticle : *pMCParticleList)
    {
      auto pdg = mcParticle->GetParticleId();

      // PRINT ONLY MUON INFO
      if(abs(pdg)!=13) continue;

      auto vertex = mcParticle->GetVertex();
      auto end_point = mcParticle->GetEndpoint();
      const long uid = (mcParticle != nullptr) ? reinterpret_cast<intptr_t>(mcParticle->GetUid()) : 0;
      auto momentum = mcParticle->GetMomentum();

      auto vertex_x = vertex.GetX();
      auto vertex_y = vertex.GetY();
      auto vertex_z = vertex.GetZ();
      auto end_point_x = end_point.GetX();
      auto end_point_y = end_point.GetY();
      auto end_point_z = end_point.GetZ();
      auto px = momentum.GetX();
      auto py = momentum.GetY();
      auto pz = momentum.GetZ();

      auto parents = mcParticle->GetParentList();
      const MCParticle* parent = (parents.size()>0)? parents.front() : nullptr;
      const long parent_uid = (parent != nullptr) ? reinterpret_cast<intptr_t>(parent->GetUid()) : 0;
      
      // PRINT ONLY PRIMARIES
      if (!parent || abs(parent->GetParticleId()) != 14) continue;
      
      auto daughters = mcParticle->GetDaughterList();
      auto nof_daughters = daughters.size();

      MCparticleInfoOutputFile
           << "{\"STAGE\" : " << "\"" << m_inputStageName << "\"" 
           << ", \"CallNumber\" : " << "\"" <<AlgoExecutionCount[m_inputStageName] << "\""
           << ", \"uid\" : " << uid
           << ", \"parent\" : " << parent_uid
           << ", \"pdg\" : " << pdg 
           << ", \"nof_daughters\" : " << nof_daughters 
           << ", \"vertex_x\" : " << vertex_x 
           << ", \"vertex_y\" : " << vertex_y 
           << ", \"vertex_z\" : " << vertex_z 
           << ", \"end_point_x\" : " << end_point_x 
           << ", \"end_point_y\" : " << end_point_y 
           << ", \"end_point_z\" : " << end_point_z 
           << ", \"px\" : " << px
           << ", \"py\" : " << py
           << ", \"pz\" : " << pz
           << "},\n";
    }
    return STATUS_CODE_SUCCESS;
}
//------------------------------------------------------------------------------------------------------------------------------------------
StatusCode PrintCurrentPfoInfoAlgorithm::PrintMCparticlesInfo(const MCParticleList*& pMCParticleList, const CaloHitList*& pMCCaloHitList) const
{
    if(!pMCParticleList) return STATUS_CODE_NOT_INITIALIZED;
    if(!pMCCaloHitList) return STATUS_CODE_NOT_INITIALIZED;
    
    LArMCParticleHelper::PrimaryParameters parameters;
    parameters.m_minHitSharingFraction = 0.f;

    LArMCParticleHelper::MCContributionMap nuMCParticlesToGoodHitsMap;
    LArMCParticleHelper::MCContributionMap beamMCParticlesToGoodHitsMap;
    LArMCParticleHelper::MCContributionMap crMCParticlesToGoodHitsMap;

    LArMCParticleHelper::SelectReconstructableMCParticles(
            pMCParticleList, pMCCaloHitList, parameters, LArMCParticleHelper::IsBeamNeutrinoFinalState, nuMCParticlesToGoodHitsMap);

    LArMCParticleHelper::SelectReconstructableMCParticles(
            pMCParticleList, pMCCaloHitList, parameters, LArMCParticleHelper::IsBeamParticle, beamMCParticlesToGoodHitsMap);
    
    LArMCParticleHelper::SelectReconstructableMCParticles(
            pMCParticleList, pMCCaloHitList, parameters, LArMCParticleHelper::IsCosmicRay, crMCParticlesToGoodHitsMap);

    std::cout << "MAP SIZES : " 
      << nuMCParticlesToGoodHitsMap.size()
      << " " << beamMCParticlesToGoodHitsMap.size() << " "
      << crMCParticlesToGoodHitsMap.size()
      << " " << pMCParticleList->size()
      << " " << pMCCaloHitList->size()
      << "\n";

    if (!nuMCParticlesToGoodHitsMap.empty())
    {
        // std::cout << std::endl << "BeamNeutrinos: " << std::endl;
        this->PrintPrimaryMCParticles(nuMCParticlesToGoodHitsMap);
    }

    if (!beamMCParticlesToGoodHitsMap.empty())
    {
        // std::cout << std::endl << "BeamParticles: " << std::endl;
        this->PrintPrimaryMCParticles(beamMCParticlesToGoodHitsMap);
    }

    if (!crMCParticlesToGoodHitsMap.empty())
    {
        // std::cout << std::endl << "CosmicRays: " << std::endl;
        this->PrintPrimaryMCParticles(crMCParticlesToGoodHitsMap);
    } 
    return STATUS_CODE_SUCCESS; 
}
//------------------------------------------------------------------------------------------------------------------------------------------
void PrintCurrentPfoInfoAlgorithm::PrintPrimaryMCParticles(const LArMCParticleHelper::MCContributionMap &mcContributionMap) const
{
    MCParticleVector mcPrimaryVector;
    LArMonitoringHelper::GetOrderedMCParticleVector({mcContributionMap}, mcPrimaryVector);

    unsigned int index(0);

    for (const MCParticle *const pMCPrimary : mcPrimaryVector)
    {
      std::cout << "mc particle " << pMCPrimary << "\n";
      const CaloHitList &caloHitList(mcContributionMap.at(pMCPrimary));
      // printing
      MCparticleInfoOutputFile
           << "{\"STAGE\" : " << "\"" << m_inputStageName << "\"" 
           << ", \"CallNumber\" : " << "\"" <<AlgoExecutionCount[m_inputStageName] << "\""
           << ", \"pMCPrimary\" : " << "\"" << pMCPrimary << "\""
           << "}, \n";

      LArMCParticleHelper::MCRelationMap mcToPrimaryMCMap;
      LArMCParticleHelper::CaloHitToMCMap caloHitToPrimaryMCMap;
      LArMCParticleHelper::MCContributionMap mcToTrueHitListMap;
      LArMCParticleHelper::GetMCParticleToCaloHitMatches(&caloHitList, mcToPrimaryMCMap, caloHitToPrimaryMCMap, mcToTrueHitListMap);
      // this->PrintMCParticle(pMCPrimary, mcToTrueHitListMap, 1);
    }
    ++index;
}
//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode PrintCurrentPfoInfoAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
  PANDORA_RETURN_RESULT_IF_AND_IF(
        STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "InputStageName", m_inputStageName));

  // hits
  PANDORA_RETURN_RESULT_IF_AND_IF(
        STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "InputCaloHitList3DName", m_inputCaloHitList3DName));
  PANDORA_RETURN_RESULT_IF_AND_IF(
        STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "InputCaloHitList2DName", m_inputCaloHitList2DName));
  PANDORA_RETURN_RESULT_IF_AND_IF(
        STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "InputCaloHitListUName", m_inputCaloHitListUName));
  PANDORA_RETURN_RESULT_IF_AND_IF(
        STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "InputCaloHitListVName", m_inputCaloHitListVName));
  PANDORA_RETURN_RESULT_IF_AND_IF(
        STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "InputCaloHitListWName", m_inputCaloHitListWName));

  // clusters
PANDORA_RETURN_RESULT_IF_AND_IF(
      STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "InputClusterListName3D", m_inputClusterListName3D));
  PANDORA_RETURN_RESULT_IF_AND_IF(
      STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "InputClusterListNameU", m_inputClusterListNameU));
  PANDORA_RETURN_RESULT_IF_AND_IF(
      STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "InputClusterListNameV", m_inputClusterListNameV));
  PANDORA_RETURN_RESULT_IF_AND_IF(
      STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "InputClusterListNameW", m_inputClusterListNameW));

  // pfos
  PANDORA_RETURN_RESULT_IF_AND_IF(
        STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadVectorOfValues(xmlHandle, "InputPfoListNames", m_inputPfoListNames));

  // CR Vertices
  PANDORA_RETURN_RESULT_IF_AND_IF(
        STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadVectorOfValues(xmlHandle, "InputVertexListNames", m_inputVertexListNames));

  // MC particles 
  PANDORA_RETURN_RESULT_IF_AND_IF(
      STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "MCParticleListNames", m_mcParticleListNames));
  PANDORA_RETURN_RESULT_IF_AND_IF(
      STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "MCCaloHitListName", m_mcCaloHitListName));


  return STATUS_CODE_SUCCESS;
}

} // namespace lar_content
