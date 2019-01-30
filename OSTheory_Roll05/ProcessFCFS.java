import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Queue;

class ProcessFCFS {

    int id;
    int numberOfPages;
    int arrivalTime;
    int actualArrival;
    Queue<Integer> memRef = new LinkedList<>();
    ArrayList<Integer> pageTable = new ArrayList<Integer>();
    int tablesize;
    int pageFault;
    int waitingTime;
    int executionTime;
    int totalTime;


    boolean isPageFault() {

        int currentmem = memRef.peek();

        if (!pageTable.contains(currentmem)) {
            return true;
        } else {

            return false;
        }

    }


    void doPageFault() {

        int currentmem = memRef.peek();

        tablesize= (numberOfPages-1)/3+1;

        if (pageTable.size() >= tablesize) {

            pageTable.remove(0);
        }
        pageTable.add(currentmem);
        pageFault++;

    }


    void noPageFault() {

        int currentmem = memRef.peek();
        memRef.remove();

        for (int j = 0; j < pageTable.size(); j++) {
            if (pageTable.get(j) == currentmem)
                pageTable.remove(j);
        }

        pageTable.add(currentmem);


    }

}
