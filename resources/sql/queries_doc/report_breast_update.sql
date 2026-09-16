UPDATE tableBreast SET
    breast_right_ecostrcture = :breast_right_ecostrcture,
    breast_right_duct        = :breast_right_duct,
    breast_right_ligament    = :breast_right_ligament,
    breast_right_formations  = :breast_right_formations,
    breast_right_ganglions   = :breast_right_ganglions,
    breast_left_ecostrcture  = :breast_left_ecostrcture,
    breast_left_duct         = :breast_left_duct,
    breast_left_ligament     = :breast_left_ligament,
    breast_left_formations   = :breast_left_formations,
    breast_left_ganglions    = :breast_left_ganglions,
    concluzion               = :concluzion,
    recommendation           = :recommendation
WHERE
    id_reportEcho = :id_reportEcho
