import java.util.Comparator;

public class FirstCome implements Comparator<ProcessFCFS> {

    @Override
    public int compare(ProcessFCFS o1, ProcessFCFS o2) {
        return (o1.arrivalTime-o2.arrivalTime);
    }
}
