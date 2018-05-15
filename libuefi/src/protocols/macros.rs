/// Implements the `Protocol` trait for a type.
macro_rules! impl_proto {
    (
        protocol $p:ident {
            GUID = $a:expr, $b:expr, $c:expr, $d:expr;
        }
    ) => (
        impl ::protocols::Protocol for $p {
            const GUID: ::Guid = ::Guid::from_values($a, $b, $c, $d);
        }

        // Most UEFI functions expect to be called on the bootstrap processor.
        impl !Send for $p {}

        // Most UEFI function do not support multithreaded access.
        impl !Sync for $p {}
    )
}
