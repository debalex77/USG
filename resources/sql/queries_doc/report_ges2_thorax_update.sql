UPDATE tableGestation2_thorax SET
    pulmonaryAreas             = :pulmonaryAreas,
    pulmonaryAreas_description = :pulmonaryAreas_description,
    pleuralCollections         = :pleuralCollections,
    diaphragm                  = :diaphragm
WHERE
    id_reportEcho = :id_reportEcho