#include <iostream>
#include <fstream>
#include <math.h>
#include "Model/Pod.h"
#include "Model/CoreSwitch.h"

using namespace std;



bool comareConnect(Connect * c1,Connect * c2)
{
    return c1->getI_ori_port()<c2->getI_ori_port();
}



int main(int argc, char*argv[]) {

    int i,k = 0, nbHostTotal,nbSwitchTotal,nbHostEachPod;
    vector<Pod> v_pods;

    if(argc>1) {
        if(k%2==0&&k<2) {
            k = atoi(argv[1]);
            nbHostTotal = pow((k / 2), 2) * k;
            nbHostEachPod = pow((k / 2), 2);
        }else{
            cout<<"Error k!"<<endl;
            return 0;
        }
    }
    ofstream outputfFile;
    outputfFile.open(" fattree.topo");

    // write information
    outputfFile<<"#fat tree topology file.\n"
            "#Value of k = "<<k<<endl<<
            "#Total number of hosts = "<<nbHostTotal<<endl<<
            "#Number of hosts under each switch = "<<k/2<<endl<<
            "####################################################\n";

    //generate pods
    for(i = 0;i<k;i++){
        Pod *p = new Pod(i,nbHostEachPod,k/2,k/2);
        v_pods.push_back(*p);
    }
    //generate hosts
    int idHost = 0;
    for(i = 0;i<v_pods.size();i++){
        int j = 0;
        for(j = 0;j<v_pods[i].getNumMaxHost();idHost++,j++){
            Host *host = new Host(idHost,1);
            v_pods[i].addHost(*host);
        }
    }
    //generate edgeSwitchs
    for(i = 0;i<v_pods.size();i++){
        int j = 0;
        //generate EdgeSwitch
        for(j = 0;j<v_pods[i].getNumMaxEageS();j++){
            int numMystery = 1;
            EdgeSwitch *edgeSwitch = new EdgeSwitch(i,j,numMystery,k);
            v_pods[i].addEageSwitch(*edgeSwitch);
        }
        //generate AggrSwitch
        for(j = 0;j<v_pods[i].getNumMaxAggrS();j++){
            int numMystery = 1;
            AggrSwitch *aggrSwitch = new AggrSwitch(i,j+2,numMystery,k);
            v_pods[i].addAggrSwitch(*aggrSwitch);
        }
    }

    //generate AggrSwitch

    for(i = 0;i<pow((k / 2), 2);i++){
        int numMystery = 1;
        CoreSwitch *coreSwitch = new CoreSwitch(k,1,numMystery,k);
    }


    vector<Pod>::iterator v_it_pod;
    multimap<string,Host>::iterator m_it_host;
    multimap<string,EdgeSwitch>::iterator m_it_es;
    multimap<string,AggrSwitch>::iterator m_it_as;

    //add connections for each pod
    for(v_it_pod = v_pods.begin();v_it_pod!=v_pods.end();v_it_pod++){
        int port_aggr = 1;//calculate the port of aggrSwitch to which the edgeSwitch is connected

        //add connection for each edgeSwitch
        for(m_it_es = v_it_pod->getV_edgeSwitch().begin();m_it_es!=v_it_pod->getV_edgeSwitch().end();m_it_es++){
            int port_edge = 1;//calculate the port of edgeSwitch to which the host is connected

            //for each edgeSwitch, connect to k/2 hosts.
            for(i = 0;i<k/2;i++) {
                int id_host = m_it_es->second.getName_a()*k+(m_it_es->second.getName_b()*k/2)+i;
                ostringstream ostr;
                ostr <<id_host;
                m_it_host = v_it_pod->getV_host().find("\"Node("+ostr.str()+")\"");
                // set edge connect to host
                Connect *connectE2H = new Connect();
                connectE2H->setPN_connect_with(&(m_it_host->second));
                connectE2H->setI_des_port(1);
                connectE2H->setI_ori_port(2*port_edge);
                m_it_es->second.addConnect(*connectE2H);

                // set host connect to edge
                Connect *connectH2E = new Connect();
                connectH2E->setI_ori_port(1);
                connectH2E->setI_des_port(2*port_edge);
                connectH2E->setPN_connect_with(&(m_it_es->second));
                m_it_host->second.addConnect(*connectH2E);
                port_edge++;
            }
            //for each edgeSwitch, connect k/2 aggrSwitch.
            //one edgeSwitch connect to all the aggrSwitch
            port_edge = 1;//calculate the port of edgeSwitch to which the aggrSwitch is connected
            for(m_it_as = v_it_pod->getV_aggrSwitch().begin();m_it_as!=v_it_pod->getV_aggrSwitch().end();m_it_as++) {
                //set edge connect to aggr, aggr's even port, edge's odd port
                Connect *connectE2A = new Connect();
                connectE2A->setPN_connect_with(&(m_it_as->second));
                connectE2A->setI_des_port(port_aggr*2);
                connectE2A->setI_ori_port(port_edge);
                m_it_es->second.addConnect(*connectE2A);

                //set aggr connect to edge
                Connect *connectA2E = new Connect();
                connectA2E->setPN_connect_with(&(m_it_es->second));
                connectA2E->setI_des_port(port_edge);
                connectA2E->setI_ori_port(port_aggr*2);
                m_it_as->second.addConnect(*connectA2E);
                port_edge+=2;
            }
            port_aggr++;
        }

    }

    //print
    //print hosts
    for(v_it_pod = v_pods.begin();v_it_pod!=v_pods.end();v_it_pod++){
        for(m_it_host = v_it_pod->getV_host().begin();m_it_host!=v_it_pod->getV_host().end();m_it_host++) {
            outputfFile<<endl;
            outputfFile << m_it_host->second.getType() << "\t";
            outputfFile << m_it_host->second.getNum_port() << "\t";
            outputfFile << m_it_host->second.getName() << endl;
            outputfFile <<"[" <<m_it_host->second.getV_connect()[0]->getI_ori_port()<<"]"<<"\t"
                        << m_it_host->second.getV_connect()[0]->getPN_connect_with()->getName()<<"\t"
                        <<"[" <<m_it_host->second.getV_connect()[0]->getI_des_port()<<"]"
                        << endl;
        }
    }

    //print edgeSwitch
    for(v_it_pod = v_pods.begin();v_it_pod!=v_pods.end();v_it_pod++){
        for(m_it_es = v_it_pod->getV_edgeSwitch().begin();m_it_es!=v_it_pod->getV_edgeSwitch().end();m_it_es++){
            outputfFile<<endl;
            outputfFile << m_it_es->second.getType() << "\t";
            outputfFile << m_it_es->second.getNum_port() << "\t";
            outputfFile << m_it_es->second.getName() << endl;
            sort(m_it_es->second.getV_connect().begin(),m_it_es->second.getV_connect().end(),comareConnect);
            for(i=0;i<m_it_es->second.getV_connect().size();i++){
                outputfFile <<"[" <<m_it_es->second.getV_connect()[i]->getI_ori_port()<<"]"<<"\t"
                            << m_it_es->second.getV_connect()[i]->getPN_connect_with()->getName()<<"\t"
                            <<"[" <<m_it_es->second.getV_connect()[i]->getI_des_port()<<"]"
                            << endl;
            }

        }

    }

    //print aggrSwitch
    for(v_it_pod = v_pods.begin();v_it_pod!=v_pods.end();v_it_pod++){
        for(m_it_as = v_it_pod->getV_aggrSwitch().begin();m_it_as!=v_it_pod->getV_aggrSwitch().end();m_it_as++){
            outputfFile<<endl;
            outputfFile<<m_it_as->second.getType()<<"\t";
            outputfFile<<m_it_as->second.getNum_port()<<"\t";
            outputfFile<<m_it_as->second.getName()<<endl;
            sort(m_it_as->second.getV_connect().begin(),m_it_as->second.getV_connect().end(),comareConnect);
            for(i=0;i<m_it_as->second.getV_connect().size();i++){
                outputfFile <<"[" <<m_it_as->second.getV_connect()[i]->getI_ori_port()<<"]"<<"\t"
                            << m_it_as->second.getV_connect()[i]->getPN_connect_with()->getName()<<"\t"
                            <<"[" <<m_it_as->second.getV_connect()[i]->getI_des_port()<<"]"
                            << endl;
            }
        }
    }

    //print coreSwitch
    for(i = 0;i<pow((k/2),2);i++){
    }
    outputfFile.close();
    return 0;
}