//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package arciprete_chiavetta_progettob.simulations;

import arciprete_chiavetta_progettob.AdHocHostFlooding;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;

network FloodingNetwork
{
    parameters:
        @display("bgb=706.58624,580.45;bgi=background/streetmap,s;bgg=0,0,#729FCF;bgu=km;i=misc/globe");
        int nodeCount;
    submodules:
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=653.0062,47.99875");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=654.1225,155.15875");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=654.1225,274.5975");
        }
        host[nodeCount]: AdHocHostFlooding {
            @display("p=388.455,247.8075;i=misc/car2;is=l");
        }
}
