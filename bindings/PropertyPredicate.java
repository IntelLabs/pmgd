/*
 * Corresponds to the Property.h file in Jarvis.
 *
 * Notes:
 *  - Omitting time, blob as an option for property types
 *  - Stubs are just in place, need to be built up for filter
 *  - need to catch and throw exceptions up
 */

public class PropertyPredicate{
    private long jarvisHandle;
    
    
    public enum op_t{ dont_care, eq, ne, gt, ge, lt, le,
            gele, gelt, gtle, gtlt};
    
    public PropertyPredicate(){
        genericPropertyPredicateNative(0, null, null, null, null);
    }
    public PropertyPredicate(String i){
        genericPropertyPredicateNative(1, i, null, null, null);
    }
    public PropertyPredicate(String i, op_t o, Property v){
        genericPropertyPredicateNative(3, i, o, v, null);
    }
    public PropertyPredicate(String i, op_t o, Property v1, Property v2){
        genericPropertyPredicateNative(3, i, o, v1, v2);
    }
    
    //Add operator overrides here
    
    private native void genericPropertyPredicateNative(int count,
                                                       String i,
                                                       op_t o,
                                                       Property v1,
                                                       Property v2);
}
