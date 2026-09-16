SELECT
    rp.docPresentationDate AS title_report,
    ln.section_type,
    ln.examinedArea,
    ln.clinicalIndications,
    ln.skin_structure,
    ln.subcutaneous_tissue,
    ln.lesion_location,
    ln.lesion_size,
    ln.lesion_echogenicity,
    ln.lesion_contour,
    ln.lesion_vascularization,
    ln.ln_number,
    ln.ln_size_nodes,
    ln.ln_shape,
    ln.ln_echogenic_hilum,
    ln.ln_cortex,
    ln.ln_structure,
    ln.ln_contour,
    ln.ln_vascularization,
    ln.ln_associated_changes,
    ln.other_changes,
    ln.concluzion,
    ln.recommendation
FROM
    reportEcho r
INNER JOIN
    reportEchoPresentation rp ON rp.id_reportEcho = r.id
INNER JOIN
    tableSofTissuesLymphNodes ln ON ln.id_reportEcho = r.id
WHERE
    r.deletionMark = 2 AND
    r.id = %id%
