import java.util.Comparator;

public class RoundRobinComp implements Comparator<ProcessRobin> {

    @Override
    public int compare(ProcessRobin o1, ProcessRobin o2) {


        if(o1.arrivalTime==o2.arrivalTime){

            if(o1.pageFaultOccured+o2.pageFaultOccured==0){
                return o2.pageFaultOccured;

            }
        }
        return (o1.arrivalTime-o2.arrivalTime);
    }
}
